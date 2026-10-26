// BC textures decoded on several cores (textures.c, Android: GPUs without BC textures - the Galaxy Tab S10 Ultra's
// Mali). BOTW streams its textures in as Link moves, and each one was decoded on the thread recording the frame, one
// 4x4 block at a time: a 2048x2048 BC1 level is a million blocks, a visible hitch. A level of at least 4096 blocks
// (256 x 256 texels) is cut into bands of block rows that a few worker threads and the caller decode together; the
// output is byte for byte the same (tools/test_bc_threads.c). The Dimensity 9300+ has eight big cores, most of them
// idle while a frame is recorded. LARECOMP_BC_THREADS=<n>: n workers (0: none, as before); default min(3, cores / 2).
// Kept free of Vulkan types so the desktop test builds it as it is.
#pragma once
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// a decoder: wb x hb blocks (bsize bytes each, rows of blocks) -> texels, rows of wb * 4 texels of bpp bytes
typedef void (*BcDecodeFn)(int fmt, const uint8_t *blocks, uint32_t wb, uint32_t hb, uint32_t bsize, uint8_t *out,
                           uint32_t bpp);

#define BCP_MAX_WORKERS 7
#define BCP_MIN_BLOCKS 4096

static struct {
    pthread_mutex_t job_lock;  // one decode at a time (held by the caller for the whole job)
    pthread_mutex_t m;         // the fields below
    pthread_cond_t go, done;
    int nworkers;              // started (-1: not decided yet)
    uint64_t gen;              // a job's number: workers wait for it to change
    int running;               // workers still on this job
    BcDecodeFn fn;
    int fmt;
    const uint8_t *blocks;
    uint8_t *out;
    uint32_t wb, hb, bsize, bpp, band;
    atomic_uint next;          // the next band to take
} bcp = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER, -1};

static void bcp_work(void) {
    uint64_t pitch = (uint64_t)bcp.wb * 4 * bcp.bpp;
    for (;;) {
        uint32_t b = atomic_fetch_add(&bcp.next, 1);
        uint64_t r0 = (uint64_t)b * bcp.band;
        if (r0 >= bcp.hb) break;
        uint32_t r1 = r0 + bcp.band > bcp.hb ? bcp.hb : (uint32_t)(r0 + bcp.band);
        bcp.fn(bcp.fmt, bcp.blocks + r0 * bcp.wb * bcp.bsize, bcp.wb, r1 - (uint32_t)r0, bcp.bsize, bcp.out + r0 * 4 * pitch,
               bcp.bpp);
    }
}

static void *bcp_thread(void *arg) {
    (void)arg;
    uint64_t seen = 0;
    pthread_mutex_lock(&bcp.m);
    for (;;) {
        while (bcp.gen == seen) pthread_cond_wait(&bcp.go, &bcp.m);
        seen = bcp.gen;
        pthread_mutex_unlock(&bcp.m);
        bcp_work();
        pthread_mutex_lock(&bcp.m);
        if (--bcp.running == 0) pthread_cond_signal(&bcp.done);
    }
    return NULL;
}

static int bcp_wanted(void) {  // (bcp.m held)
    const char *e = getenv("LARECOMP_BC_THREADS");
    long cores = sysconf(_SC_NPROCESSORS_ONLN);
    int n = e ? atoi(e) : (int)(cores / 2 < 3 ? cores / 2 : 3);
    return n < 0 ? 0 : n > BCP_MAX_WORKERS ? BCP_MAX_WORKERS : n;
}

static void bc_decode_parallel(BcDecodeFn fn, int fmt, const uint8_t *blocks, uint32_t wb, uint32_t hb, uint32_t bsize,
                               uint8_t *out, uint32_t bpp) {
    if ((uint64_t)wb * hb < BCP_MIN_BLOCKS || hb < 8) { fn(fmt, blocks, wb, hb, bsize, out, bpp); return; }
    pthread_mutex_lock(&bcp.job_lock);
    pthread_mutex_lock(&bcp.m);
    if (bcp.nworkers < 0) {  // (the first big level: the workers start, for good)
        int want = bcp_wanted();
        bcp.nworkers = 0;
        for (int i = 0; i < want; i++) {
            pthread_t t;
            if (pthread_create(&t, NULL, bcp_thread, NULL) != 0) break;
            pthread_detach(t);
            bcp.nworkers++;
        }
        fprintf(stderr, "[vk] BC textures decoded on %d worker thread%s and the caller\n", bcp.nworkers,
                bcp.nworkers == 1 ? "" : "s");
    }
    if (!bcp.nworkers) {
        pthread_mutex_unlock(&bcp.m);
        pthread_mutex_unlock(&bcp.job_lock);
        fn(fmt, blocks, wb, hb, bsize, out, bpp);
        return;
    }
    bcp.fn = fn, bcp.fmt = fmt, bcp.blocks = blocks, bcp.out = out;
    bcp.wb = wb, bcp.hb = hb, bcp.bsize = bsize, bcp.bpp = bpp;
    uint32_t parts = (uint32_t)(bcp.nworkers + 1) * 4;  // (a few bands each: the faster cores take more)
    bcp.band = (hb + parts - 1) / parts;
    if (bcp.band < 2) bcp.band = 2;
    atomic_store(&bcp.next, 0);
    bcp.running = bcp.nworkers;
    bcp.gen++;
    pthread_cond_broadcast(&bcp.go);
    pthread_mutex_unlock(&bcp.m);
    bcp_work();
    pthread_mutex_lock(&bcp.m);
    while (bcp.running > 0) pthread_cond_wait(&bcp.done, &bcp.m);
    pthread_mutex_unlock(&bcp.m);
    pthread_mutex_unlock(&bcp.job_lock);
}
