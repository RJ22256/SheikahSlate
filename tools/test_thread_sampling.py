#!/usr/bin/env python3
"""The performance capture's thread sampler (runtime/platform/android.c plat_thread_sample) on the desktop: cut out of
the file, built with stubs for the rest of the runtime, run against two busy threads (one in a loop of its own, one
in memcpy) and a sleeping one for 1 s. Checks the report names the busy threads with most of a core each, finds the
loop and memcpy (libc's implementation, by name) among their hottest places, puts the libc time down to the function
that called memcpy, leaves the sleeper unsampled-idle, and that the process survives signals arriving after the
capture. Usage: python3 tools/test_thread_sampling.py [cc]   (Linux)"""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'build' / 'thread-sampling-test'
out.mkdir(parents=True, exist_ok=True)
src = (root / 'runtime/platform/android.c').read_text()
code = src[src.index('// ---- thread sampling (Sheikah Slate'):]
harness = r'''
#define _GNU_SOURCE
#include <dirent.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/ucontext.h>
#include <time.h>
#include <unistd.h>
#define LOG(...) (fprintf(stdout, __VA_ARGS__), fflush(stdout))
typedef struct { int ncluster; uint32_t cluster_mhz[8]; float soc_temp_c; } PlatDeviceStats;
static void plat_device_stats(PlatDeviceStats *s) { memset(s, 0, sizeof *s); s->soc_temp_c = -1; }
static uint64_t plat_now_ns(void) { struct timespec t; clock_gettime(CLOCK_MONOTONIC, &t); return (uint64_t)t.tv_sec * 1000000000ull + (uint64_t)t.tv_nsec; }
static uint64_t plat_wt_faults(void) { return 0; }
static int32_t s_presenting_tid, s_render_tid;
''' + code + r'''
__attribute__((noinline)) void busy_loop_here(void) {
    volatile uint64_t x = 0;
    for (;;) x = x * 6364136223846793005ull + 1442695040888963407ull;
}
static void *spin(void *a) { (void)a; s_presenting_tid = (int32_t)syscall(SYS_gettid); busy_loop_here(); return NULL; }
void *copier(void *a) {  // (exported: named in the report, as libmain.so's functions are)
    (void)a;
    s_render_tid = (int32_t)syscall(SYS_gettid);
    static char x[1 << 20], y[1 << 20];
    void *(*volatile mc)(void *, const void *, size_t) = memcpy;  // (libc's, not an inlined copy)
    for (;;) { mc(x, y, sizeof x); mc(y, x, sizeof y); }
    return NULL;
}
static void *sleeper(void *a) { (void)a; for (;;) { struct timespec t = {0, 5000000}; nanosleep(&t, NULL); } return NULL; }
int main(void) {
    pthread_t a, b, c, idle[80];
    for (int i = 0; i < 80; i++) pthread_create(&idle[i], NULL, sleeper, NULL);  // (the game has dozens: the busy ones come after)
    pthread_create(&a, NULL, spin, NULL);
    pthread_create(&b, NULL, copier, NULL);
    pthread_create(&c, NULL, sleeper, NULL);
    pthread_setname_np(a, "game-main");
    pthread_setname_np(b, "renderer");
    pthread_setname_np(c, "sleeper");
    struct timespec t = {0, 200000000};
    nanosleep(&t, NULL);
    plat_thread_sample(1000);
    t.tv_sec = 2; t.tv_nsec = 0;
    nanosleep(&t, NULL);
    for (int i = 0; i < 200; i++) raise(SIGPROF);  // (late signals: the handler, off, must not kill the process)
    plat_thread_sample(300);  // (a second capture)
    t.tv_sec = 1; nanosleep(&t, NULL);
    puts("survived");
    return 0;
}
'''
(out / 'harness.c').write_text(harness)
cc = sys.argv[1] if len(sys.argv) > 1 else 'clang'
subprocess.run([cc, '-O1', '-g', '-rdynamic', '-Wall', '-Wno-unused-function', '-o', str(out / 'harness'), str(out / 'harness.c'),
                '-lpthread', '-lrt', '-ldl'], check=True)
r = subprocess.run([str(out / 'harness')], capture_output=True, text=True, timeout=60)
print(r.stdout)
fails = 0
def check(cond, what):
    global fails
    print(('ok  ' if cond else 'FAIL'), what)
    fails += not cond
import re
lines = r.stdout.splitlines()
check(r.returncode == 0 and 'survived' in r.stdout, 'the process survives late signals and a second capture')
gm = [l for l in lines if 'game-main' in l and 'of a core' in l]
rd = [l for l in lines if 'renderer' in l and 'of a core' in l]
check(bool(gm) and int(re.search(r'(\d+)% of a core', gm[0]).group(1)) >= 70, 'the spinning thread: most of a core')
check(bool(rd) and int(re.search(r'(\d+)% of a core', rd[0]).group(1)) >= 70, 'the copying thread: most of a core')
check(any('busy_loop_here' in l for l in lines), 'the spinning thread\'s loop among its hottest places')
check(bool(rd) and float(re.search(r'libc (\d+)%', rd[0]).group(1)) >= 50, 'the copying thread: mostly in libc')
check(any('hottest' in l and re.search(r'libc\.so[.0-9]*!memcpy', l) for l in lines), 'memcpy named (the implementation libc picked)')
called = [l for l in lines if 'libc called from' in l]
check(bool(called) and 'copier' in called[0], 'the libc time put down to its caller (copier)')
check(not any('sleeper' in l and 'of a core' in l for l in lines), 'the sleeping thread under 2%: not listed')
check(any('(presents frames)' in l for l in lines) and any('(renderer)' in l for l in lines), 'the presenting / render threads tagged')
print('all passed' if not fails else f'{fails} FAILED')
sys.exit(fails != 0)
