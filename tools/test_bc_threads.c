// The BC decode spread over threads (runtime/nvn/vulkan/bc_threads.h) against the plain one, on the desktop: every BC
// format, big / odd / small levels, many jobs in a row (races) - the output must be byte for byte the same.
// textures.c's own bc_decode is used, cut out of the file (it needs Vulkan's format numbers only).
//   python3 tools/test_bc_threads.py   (extracts it and builds this)
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum {
    VK_FORMAT_BC1_RGB_UNORM_BLOCK = 131, VK_FORMAT_BC1_RGB_SRGB_BLOCK, VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
    VK_FORMAT_BC1_RGBA_SRGB_BLOCK, VK_FORMAT_BC2_UNORM_BLOCK, VK_FORMAT_BC2_SRGB_BLOCK, VK_FORMAT_BC3_UNORM_BLOCK,
    VK_FORMAT_BC3_SRGB_BLOCK, VK_FORMAT_BC4_UNORM_BLOCK, VK_FORMAT_BC4_SNORM_BLOCK, VK_FORMAT_BC5_UNORM_BLOCK,
    VK_FORMAT_BC5_SNORM_BLOCK, VK_FORMAT_BC6H_UFLOAT_BLOCK, VK_FORMAT_BC6H_SFLOAT_BLOCK, VK_FORMAT_BC7_UNORM_BLOCK,
    VK_FORMAT_BC7_SRGB_BLOCK
} VkFormat;

#define BCDEC_STATIC
#define BCDEC_IMPLEMENTATION
#define BCDEC_BC4BC5_PRECISE
#include "../runtime/nvn/bcdec.h"
#include "bc_decode_extracted.inc"  // (textures.c bc_decode, bc_decode_fmt; then bc_threads.h)

static double now(void) {
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec / 1e9;
}

int main(void) {
    static const struct { VkFormat f; uint32_t bsize, bpp; const char *name; } F[] = {
        {VK_FORMAT_BC1_RGBA_UNORM_BLOCK, 8, 4, "BC1"}, {VK_FORMAT_BC2_UNORM_BLOCK, 16, 4, "BC2"},
        {VK_FORMAT_BC3_SRGB_BLOCK, 16, 4, "BC3"}, {VK_FORMAT_BC4_UNORM_BLOCK, 8, 1, "BC4"},
        {VK_FORMAT_BC4_SNORM_BLOCK, 8, 1, "BC4s"}, {VK_FORMAT_BC5_SNORM_BLOCK, 16, 2, "BC5s"},
        {VK_FORMAT_BC6H_UFLOAT_BLOCK, 16, 8, "BC6H"}, {VK_FORMAT_BC7_UNORM_BLOCK, 16, 4, "BC7"}};
    static const uint32_t S[][2] = {{512, 512}, {301, 77}, {40, 40}, {4096, 2}, {1, 4096}, {64, 64}};
    int fails = 0;
    double t_serial = 0, t_parallel = 0;
    srand(12345);
    for (size_t fi = 0; fi < sizeof F / sizeof *F; fi++)
        for (size_t si = 0; si < sizeof S / sizeof *S; si++) {
            uint32_t wb = S[si][0], hb = S[si][1], bs = F[fi].bsize, bpp = F[fi].bpp;
            size_t in_n = (size_t)wb * hb * bs, out_n = (size_t)wb * 4 * hb * 4 * bpp;
            uint8_t *in = malloc(in_n), *a = malloc(out_n), *b = malloc(out_n);
            for (size_t i = 0; i < in_n; i++) in[i] = (uint8_t)rand();
            memset(a, 0xAA, out_n);
            memset(b, 0x55, out_n);
            double t0 = now();
            bc_decode(F[fi].f, in, wb, hb, bs, a, bpp);
            double t1 = now();
            for (int rep = 0; rep < 3; rep++) bc_decode_parallel(bc_decode_fmt, (int)F[fi].f, in, wb, hb, bs, b, bpp);
            double t2 = now();
            if (wb == 512 && hb == 512) t_serial += t1 - t0, t_parallel += (t2 - t1) / 3;
            int same = !memcmp(a, b, out_n);
            if (!same) fails++;
            printf("%s %-4s %4ux%-4u blocks: %s\n", same ? "ok  " : "FAIL", F[fi].name, wb, hb, same ? "identical" : "DIFFERENT");
            free(in), free(a), free(b);
        }
    printf("2048x2048-texel levels, all formats: one thread %.0f ms, with the workers %.0f ms (%.1fx)\n", t_serial * 1e3,
           t_parallel * 1e3, t_serial / t_parallel);
    printf(fails ? "%d FAILED\n" : "all passed\n", fails);
    return fails != 0;
}
