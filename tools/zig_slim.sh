#!/bin/bash
# usage: tools/zig_slim.sh <game code flags>: the Windows release's compiler, build/zig-slim/ (shipped as zig/), made
# from tools/fetch_zig.sh's build/zig/. The full zig compiles the MinGW C runtime and its own runtime library from
# ~4000 source files on the player's machine at the first link; here they are built once, ahead of time, and only
# what building the game code reads is kept:
#   zig.exe, lib/std/std.zig (how zig finds its lib folder), the headers the game code includes (from the compiler's
#   dependency lists of a probe), rt/ = the runtime libraries a `zig cc -shared` link uses and rt/link.rsp = that
#   link's flags and libraries (runtime/gamebuild.c links with `zig lld-link @rt/link.rsp -LIBPATH:rt`).
# Checked at the end: a probe compiled and linked with the slim copy and an empty cache builds nothing from source.
set -euo pipefail
cd "$(dirname "$0")/.."
Z=build/zig O=build/zig-slim W=build/zig-slim-work OA=$PWD/build/zig-slim
[ -x $Z/zig.exe ] || { echo "tools/fetch_zig.sh first (the compiler)"; exit 1; }
rm -rf $O $W
mkdir -p $W/cache $O/lib/std $O/rt
export ZIG_GLOBAL_CACHE_DIR="$(cygpath -w "$PWD/$W/cache")" ZIG_LOCAL_CACHE_DIR="$(cygpath -w "$PWD/$W/cache")"
mkdir -p $W/tmp
export TMPDIR="$(cygpath -w "$PWD/$W/tmp")" TMP="$(cygpath -w "$PWD/$W/tmp")" TEMP="$(cygpath -w "$PWD/$W/tmp")"  # (MSYS's /c/... TMPDIR: not for zig)
flags=()  # (the game code's flags as gamebuild.c add_cflags passes them to zig)
for f in "$@"; do
    case $f in -march=*) c=${f#-march=}; flags+=("-mcpu=${c//-/_}") ;; *) flags+=("$f") ;; esac
done

# 1. the headers: what the generated C (cpu.h, simd.h; <setjmp.h> where BOTW calls the guest setjmp - tools/gen.py
#    setjmp_call) and tools/gamelib_entry.c include. (0.2.8-pc.1 left setjmp.h out: the first chunk that calls the guest
#    setjmp didn't compile on players' PCs - "the game code didn't compile")
cat > $W/probe.c <<'PROBE'
#include "cpu.h"
#include "simd.h"
#include <setjmp.h>
jmp_buf *rt_setjmp_slot(Cpu *c, uint64_t gbuf);
void probe_sj(Cpu *c) { jmp_buf *hb_ = rt_setjmp_slot(c, c->x[0]); if (!setjmp(*hb_)) c->x[0] = 1; }
PROBE
$Z/zig.exe cc "${flags[@]}" -Iruntime -c $W/probe.c -o $W/probe.o -MD -MF $W/probe.d
$Z/zig.exe cc "${flags[@]}" -Iruntime '-DGAMELIB_ID="0"' -c tools/gamelib_entry.c -o $W/entry.o -MD -MF $W/entry.d
n=0
for h in $(cat $W/probe.d $W/entry.d | tr ' ' '\n' | tr -d '\r' | sed -n 's|^build/zig/lib/|lib/|p' | sort -u); do
    mkdir -p "$O/$(dirname "$h")"
    cp "$Z/$h" "$O/$h"
    n=$((n + 1))
done
cp $Z/zig.exe $Z/LICENSE $O/
cp $Z/lib/std/std.zig $O/lib/std/

# 2. the runtime: a probe DLL importing from the exe, linked the way the full zig does it (-v: its lld-link line);
#    -s: the runtime is built without debug info (it would name this machine's folders)
#    (it calls what the game code does: setjmp, the C library's string and math functions)
printf 'LIBRARY WSword.exe\nEXPORTS\nprobe_host\nprobe_buf\n' > $W/host.def
cat > $W/dll.c <<'PROBE'
#include <math.h>
#include <setjmp.h>
#include <string.h>
void probe_host(void);
jmp_buf *probe_buf(void);
__declspec(dllexport) double probe(double x, void *d, const void *s, unsigned n) {
    if (!setjmp(*probe_buf())) probe_host();
    memcpy(d, s, n);
    memmove(d, s, n);
    return fma(x, x, 1) + sqrt(x) + nearbyint(x) + trunc(x) + floor(x) + ceil(x) + round(x) + fmaf((float)x, 2, 1);
}
PROBE
ZA=$PWD/$Z/zig.exe
(cd $W && $ZA dlltool -d host.def -l host.a -m i386:x86-64 && $ZA cc "${flags[@]}" -c dll.c -o dll.o &&
    $ZA cc -shared -s -o probe.dll dll.o host.a -v 2> link.log) || { echo "tools/zig_slim.sh: probe link failed ($W/link.log)"; exit 1; }
line=$(grep '^lld-link ' $W/link.log | tail -1 | tr -d '\r')
[ -n "$line" ] || { echo "tools/zig_slim.sh: no lld-link line in $W/link.log"; exit 1; }
: > $O/rt/link.rsp
for t in $line; do
    case $t in
        lld-link) ;;
        -OUT:*|-PDB:*|-PDBALTPATH:*|-DEBUG|-IMPLIB:*) ;;  # (gamebuild.c: -OUT; no debug info, no import library)
        -*) echo "$t" >> $O/rt/link.rsp ;;
        *ubsan_rt*) ;;  # (the sanitizer runtime of an unoptimized link: the game code is built with -O1)
        *.lib|*.obj)
            if [[ $t == *cache* ]]; then  # (paths relative to the folder the probe linked in)
                p=$(cygpath -u "$t"); [[ $p == /* ]] || p=$W/$p
                cp "$p" $O/rt/
                basename "$p" >> $O/rt/link.rsp
            fi ;;
    esac
done
echo "-NOIMPLIB" >> $O/rt/link.rsp

# 3. check: the probe again with the slim copy and an empty cache
rm -rf $W/cache2
export ZIG_GLOBAL_CACHE_DIR="$(cygpath -w "$PWD/$W/cache2")" ZIG_LOCAL_CACHE_DIR="$(cygpath -w "$PWD/$W/cache2")"
$O/zig.exe cc "${flags[@]}" -Iruntime -c $W/probe.c -o $W/probe2.o
$O/zig.exe cc "${flags[@]}" -Iruntime '-DGAMELIB_ID="0"' -c tools/gamelib_entry.c -o $W/entry2.o
(cd $W && "$OA/zig.exe" lld-link "@$(cygpath -w "$OA/rt/link.rsp")" "-LIBPATH:$(cygpath -w "$OA/rt")" \
    -OUT:probe2.dll dll.o host.a) || { echo "tools/zig_slim.sh: the slim zig's link failed"; exit 1; }
# 4. (WSword) the game code as the player's PC builds it (runtime/gamebuild.c): a chunk shaped like tools/gen.py's -
#    the guest setjmp wrapped in a host one, tail calls, the runtime's functions, the FP/SIMD helpers - the function
#    tables and tools/gamelib_entry.c, compiled with the slim zig and the game code's flags, linked with rt/link.rsp
#    against the release's own import library ($IMPLIB: the exe really exports what the game code calls)
if [ -n "${IMPLIB:-}" ]; then
    cat > $W/chunk.c <<'PROBE'
#include "cpu.h"
#include "simd.h"
#include <setjmp.h>
jmp_buf *rt_setjmp_slot(Cpu *c, uint64_t gbuf);
void f_7000001000(Cpu *c);
void hle_longjmp(Cpu *c);
void f_7000002000(Cpu *c) {
    FN_ENTER(0x7000002000);
    { jmp_buf *hb_ = rt_setjmp_slot(c, c->x[0]); if (!setjmp(*hb_)) { c->x[30] = 0x7000002010; f_7000001000(c); } }
    if (c->x[3]) { hle_longjmp(c); return; }
    if (c->x[1]) { c->pc = 0x7000003000; __attribute__((musttail)) return rt_dispatch(c); }
    __attribute__((musttail)) return f_7000001000(c);
}
void f_7000001000(Cpu *c) {
    if (c->x[2] == 7) rt_udf(c, 0x7000001000);
    if (c->x[2] == 8) rt_svc(c, 0x26, 0x7000001004);
    c->x[0] = rt_ticks() + LD64(c->x[4]) + udiv64(c->x[5], c->x[6]);
    ST32(c->x[4], (uint32_t)c->x[0]);
}
PROBE
    printf '#include "cpu.h"
void f_7000001000(Cpu *c);
void f_7000002000(Cpu *c);
const uint64_t main_fn_addrs[] = {0x7000001000, 0x7000002000};
const GuestFn main_fn_ptrs[] = {f_7000001000, f_7000002000};
const uint64_t main_fn_count = 2;
' > $W/main_table.c
    printf '#include "cpu.h"
const uint64_t sdk_fn_addrs[] = {0};
const GuestFn sdk_fn_ptrs[] = {0};
const uint64_t sdk_fn_count = 0;
' > $W/sdk_table.c
    for c in chunk main_table sdk_table; do
        $O/zig.exe cc "${flags[@]}" -Iruntime '-DGAMELIB_ID="0"' -c $W/$c.c -o $W/$c.o || { echo "tools/zig_slim.sh: $c.c didn't compile with the slim zig"; exit 1; }
    done
    (cd $W && "$OA/zig.exe" lld-link "@$(cygpath -w "$OA/rt/link.rsp")" "-LIBPATH:$(cygpath -w "$OA/rt")" \
        -OUT:botwgame_probe.dll chunk.o main_table.o sdk_table.o entry2.o "$(cygpath -w "$OLDPWD/$IMPLIB")") ||
        { echo "tools/zig_slim.sh: a game code probe didn't link against $IMPLIB"; exit 1; }
    echo "game code probe: compiled and linked against $IMPLIB"
fi
built=$(find $W/cache2 -name '*.lib' | wc -l)
[ "$built" = 0 ] || { echo "tools/zig_slim.sh: the slim zig still built $built libraries"; exit 1; }
echo "build/zig-slim: $n headers, $(ls $O/rt | wc -l) runtime files, $(find $O -type f | wc -l) files, $(du -sh $O | cut -f1)"
