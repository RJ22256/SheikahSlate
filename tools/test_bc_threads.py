#!/usr/bin/env python3
"""Builds and runs tools/test_bc_threads.c with textures.c's own bc_decode (cut out of the file) and bc_threads.h.
Usage: python3 tools/test_bc_threads.py [cc]   (default: clang; needs pthreads)"""
from pathlib import Path
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
out = root / 'build' / 'bc-threads-test'
out.mkdir(parents=True, exist_ok=True)
src = (root / 'runtime/nvn/vulkan/textures.c').read_text()
start = src.index('// wb x hb blocks (bsize bytes each, rows of blocks) -> texels')
end = src.index('#include "bc_threads.h"')
inc = src[start:end] + '#include "' + str(root / 'runtime/nvn/vulkan/bc_threads.h') + '"\n'
(out / 'bc_decode_extracted.inc').write_text(inc)
cc = sys.argv[1] if len(sys.argv) > 1 else 'clang'
exe = out / 'test_bc_threads'
subprocess.run([cc, '-O2', '-Wall', '-Wno-unused-function', '-I', str(out), '-o', str(exe),
                str(root / 'tools/test_bc_threads.c'), '-lpthread'], check=True)
sys.exit(subprocess.run([str(exe)], env={'LARECOMP_BC_THREADS': '3'}).returncode)
