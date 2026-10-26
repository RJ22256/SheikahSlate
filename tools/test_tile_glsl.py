#!/usr/bin/env python3
"""On-tile reads (exec.c tile_glsl): the fragment-shader rewrite, compiled for Vulkan with glslang.
Takes exec.c's own glsl_ident / tile_rename / tile_glsl (cut out of the file) and translate.c's GLSL prelude, rewrites
a fragment shader shaped like the translator's (texture / textureLod / texelFetch / textureGather / tsize lookups on
several slots, early fragment tests, outputs on several targets), checks what was renamed, and compiles the original
and the variants with glslangValidator (Vulkan 1.3): a variant that doesn't compile would fall back to the plain shader
on the tablet. Usage: python3 tools/test_tile_glsl.py [glslangValidator] [cc]   (skipped without glslangValidator)"""
from pathlib import Path
import os
import re
import shutil
import subprocess
import sys

root = Path(__file__).resolve().parents[1]
glslang = sys.argv[1] if len(sys.argv) > 1 else shutil.which('glslangValidator')
cc = sys.argv[2] if len(sys.argv) > 2 else 'clang'
if not glslang or not os.path.exists(glslang):
    print('glslangValidator not found: skipped')
    sys.exit(0)
out = root / 'build' / 'tile-glsl-test'
out.mkdir(parents=True, exist_ok=True)
ex = (root / 'runtime/nvn/vulkan/exec.c').read_text()
fn = ex[ex.index('// The next identifier in src at *i'):ex.index('// Fragment modules with on-tile reads')]
tr = (root / 'runtime/nvn/maxwell/translate.c').read_text()
m = re.search(r'static const char glsl_prelude\[\] =\n(.*?);\n', tr, re.S)
prelude = m.group(1)

body = r'''
  vec4 f = gl_FragCoord;
  vec2 uv = vec2(f.x / 1600.0f, f.y / 900.0f);
  vec4 a = texture(t0, vec2(uv.x, uv.y));
  vec4 b = texture(t0, vec2(uv.x, uv.y), -0.5f);
  vec4 c = textureLod(t0, vec2(uv.x, uv.y), 0.0f);
  vec4 d = texelFetch(t0, ivec2(uvec2(uint(f.x), uint(f.y))), 0);
  vec4 e = texture( t2 , vec2(uv.x, uv.y));
  vec4 g = textureGather(t0, vec2(uv.x, uv.y), 1);
  vec4 h = texture(t1, vec2(uv.x, uv.y));
  vec4 k = texture(t10, vec2(uv.x, uv.y));
  ivec3 sz = tsize0(0);
  c0 = a + b + c + d + g + h + k + vec4(float(sz.x));
  c1 = e;
  c3 = a * e;
'''
harness = r'''
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
''' + fn + r'''
static const char prelude[] =
''' + prelude + r''';
int main(void) {
    char src[16384];
    int n = snprintf(src, sizeof src, "#version 460\n%s", prelude);
    for (int i = 0; i < 11; i++) {
        if (i > 2 && i != 10) continue;
        n += snprintf(src + n, sizeof src - n, "layout(set = 1, binding = %d) uniform sampler2D t%d;\n"
                      "ivec3 tsize%d(int l) { return ivec3(textureSize(t%d, l), 1); }\n", 32 + i, i, i, i);
    }
    n += snprintf(src + n, sizeof src - n, "layout(location = 0) out vec4 c0;\nlayout(location = 1) out vec4 c1;\n"
                  "layout(location = 3) out vec4 c3;\nlayout(early_fragment_tests) in;\nvoid main() {\n%s}\n", BODY);
    FILE *f = fopen("orig.frag", "w"); fputs(src, f); fclose(f);
    uint8_t s1[] = {0}, l1[] = {1}, s2[] = {0, 2}, l2[] = {1, 3}, s3[] = {0, 2}, l3[] = {1, 1}, s4[] = {7}, l4[] = {0};
    char *v1 = tile_glsl(src, 1, s1, l1), *v2 = tile_glsl(src, 2, s2, l2), *v3 = tile_glsl(src, 2, s3, l3), *v4 = tile_glsl(src, 1, s4, l4);
    if (!v1 || !v2 || !v3) { puts("FAIL: no variant"); return 1; }
    if (v4) { puts("FAIL: a variant for a slot the shader doesn't read"); return 1; }
    f = fopen("tile1.frag", "w"); fputs(v1, f); fclose(f);
    f = fopen("tile2.frag", "w"); fputs(v2, f); fclose(f);
    f = fopen("tile3.frag", "w"); fputs(v3, f); fclose(f);
    uint8_t one = 0, ten = 10, two = 2, one1 = 1;
    printf("counts: slot0 %d slot10 %d slot2 %d slot1 %d\n", tile_rename(src, 1, &one, NULL), tile_rename(src, 1, &ten, NULL),
           tile_rename(src, 1, &two, NULL), tile_rename(src, 1, &one1, NULL));
    return 0;
}
'''
harness = harness.replace('BODY', '"' + body.replace('\\', '\\\\').replace('"', '\\"').replace('\n', '\\n') + '"')
(out / 'harness.c').write_text(harness)
subprocess.run([cc, '-O1', '-w', '-o', str(out / 'harness'), str(out / 'harness.c')], check=True)
r = subprocess.run([str(out / 'harness')], cwd=out, capture_output=True, text=True)
print(r.stdout.strip())
if r.returncode: sys.exit(1)
fails = 0
if 'counts: slot0 4 slot10 1 slot2 1 slot1 1' not in r.stdout:  # (texture x2, textureLod, texelFetch on t0; t10 not t1)
    print('FAIL: rename counts'); fails += 1
v1 = (out / 'tile1.frag').read_text()
for want in ['tl0_tex(vec2(uv.x, uv.y))', 'tl0_tex(vec2(uv.x, uv.y), -0.5f)', 'tl0_lod(vec2(uv.x, uv.y), 0.0f)',
             'tl0_fetch(ivec2(', 'textureGather(t0,', 'texture(t1, vec2', 'texture(t10, vec2', 'texture( t2 , vec2',
             'textureSize(t0, l)', '#extension GL_EXT_shader_tile_image : require',
             'layout(location = 1) tileImageEXT highp attachmentEXT tl_img1;']:
    if want not in v1:
        print('FAIL: variant 1 lacks', repr(want)); fails += 1
if v1.index('#extension GL_EXT_shader_tile_image') > v1.index('uniform sampler2D'):
    print('FAIL: the extension after declarations'); fails += 1
v3 = (out / 'tile3.frag').read_text()
if v3.count('tileImageEXT') != 1:
    print('FAIL: two slots on one target declared it twice'); fails += 1
for name in ['orig', 'tile1', 'tile2', 'tile3']:
    c = subprocess.run([glslang, '--target-env', 'vulkan1.3', '-S', 'frag', '-o', name + '.spv', name + '.frag'], cwd=out,
                       capture_output=True, text=True)
    ok = c.returncode == 0
    print(('ok  ' if ok else 'FAIL'), name, 'compiles for Vulkan 1.3' if ok else c.stdout.strip()[-600:])
    fails += not ok
print('all passed' if not fails else f'{fails} FAILED')
sys.exit(fails != 0)
