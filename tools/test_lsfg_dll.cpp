// WSword: the Lossless.dll check (third_party/lsfg-vk checkShaderDll, behind Settings > Lossless Scaling) against
// made-up DLLs - a PE32+ file with an RT_RCDATA directory as Lossless Scaling's has, its resources numbered as the
// backend looks them up (49 + id, +23 for the performance set, +49 for full precision). No real Lossless.dll is used
// or needed. Build: tools/test_lsfg_dll.sh (CI's desktop tests).
// SPDX-License-Identifier: GPL-3.0-or-later (it links lsfg-vk)
#include "lsfg-vk-backend/lsfgvk.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <set>
#include <string>
#include <vector>

namespace {
void put16(std::vector<uint8_t> &b, size_t at, uint16_t v) { memcpy(&b[at], &v, 2); }
void put32(std::vector<uint8_t> &b, size_t at, uint32_t v) { memcpy(&b[at], &v, 4); }

// A PE32+ DLL whose .rsrc holds these RT_RCDATA ids (and two other resource types beside it)
std::vector<uint8_t> make_dll(const std::set<uint32_t> &ids) {
    const uint32_t RSRC_RVA = 0x1000, RSRC_OFF = 0x200;
    std::vector<uint8_t> r;  // the resource section
    auto grow = [&r](size_t n) { size_t at = r.size(); r.resize(at + n); return at; };
    // root: 3 types (RT_ICON 3, RT_RCDATA 10, RT_VERSION 16)
    size_t root = grow(16 + 3 * 8);
    put16(r, root + 14, 3);
    size_t empty = grow(16);  // (the other two types: an empty directory)
    size_t rc = grow(16 + ids.size() * 8);
    put16(r, rc + 14, (uint16_t)ids.size());
    const uint32_t types[3] = {3, 10, 16};
    for (int i = 0; i < 3; i++) {
        put32(r, root + 16 + i * 8, types[i]);
        put32(r, root + 20 + i * 8, 0x80000000u | (uint32_t)(types[i] == 10 ? rc : empty));
    }
    size_t k = 0;
    for (uint32_t id : ids) {
        size_t lang = grow(16 + 8), entry = grow(16), data = grow(8);
        put16(r, lang + 14, 1);
        put32(r, lang + 16, 0x409);
        put32(r, lang + 20, (uint32_t)entry);  // (a data entry: no high bit)
        put32(r, entry, RSRC_RVA + (uint32_t)data);
        put32(r, entry + 4, 8);
        put32(r, data, 0x07230203);  // (SPIR-V's magic, for the look of it)
        put32(r, data + 4, id);
        put32(r, rc + 16 + k * 8, id);
        put32(r, rc + 20 + k * 8, 0x80000000u | (uint32_t)lang);
        k++;
    }
    std::vector<uint8_t> f(RSRC_OFF + r.size());
    put16(f, 0, 0x5A4D);
    put32(f, 0x3C, 0x40);
    put32(f, 0x40, 0x00004550);  // PE\0\0
    put16(f, 0x40 + 6, 1);        // one section
    put16(f, 0x40 + 20, 240);     // PE32+ optional header size
    const size_t opt = 0x40 + 24;
    put16(f, opt, 0x20B);
    put32(f, opt + 128, RSRC_RVA);  // data directory 2: resources
    put32(f, opt + 132, (uint32_t)r.size());
    const size_t sec = opt + 240;
    memcpy(&f[sec], ".rsrc\0\0\0", 8);
    put32(f, sec + 8, (uint32_t)r.size());
    put32(f, sec + 12, RSRC_RVA);
    put32(f, sec + 16, (uint32_t)r.size());
    put32(f, sec + 20, RSRC_OFF);
    memcpy(&f[RSRC_OFF], r.data(), r.size());
    return f;
}

std::set<uint32_t> shader_set(bool fp16) {
    std::set<uint32_t> s;
    for (uint32_t id = 255; id <= 279; id++) {
        uint32_t base = 49 + id + (fp16 ? 0 : 49);
        s.insert(base);
        if (id >= 257) s.insert(base + 23);
    }
    return s;
}

int fails;
void expect(const char *what, const std::vector<uint8_t> &dll, int want) {
    const char *path = "build/test_lsfg.dll";
    std::ofstream(path, std::ios::binary).write((const char *)dll.data(), (std::streamsize)dll.size());
    int got;
    std::string err;
    try {
        got = lsfgvk::backend::checkShaderDll(path);
    } catch (const std::exception &e) {
        got = -1;
        err = e.what();
    }
    bool ok = got == want;
    printf("%s %-46s sets %d (want %d)%s%s\n", ok ? "ok  " : "FAIL", what, got, want, err.empty() ? "" : ": ",
           err.substr(0, err.find('\n')).c_str());
    if (!ok) fails++;
}
}  // namespace

int main() {
    std::set<uint32_t> both = shader_set(false), fp16 = shader_set(true);
    both.insert(fp16.begin(), fp16.end());
    both.insert(5);  // (unrelated resources beside them)
    expect("both sets", make_dll(both), 3);
    expect("full precision only", make_dll(shader_set(false)), 1);
    expect("FP16 only", make_dll(shader_set(true)), 2);
    std::set<uint32_t> gap = shader_set(false);
    gap.erase(49 + 49 + 268 + 23);  // (one performance shader missing)
    expect("full precision set with a shader missing", make_dll(gap), -1);
    expect("no shaders", make_dll({1, 2, 3}), -1);
    std::vector<uint8_t> junk(4096, 0x41);
    expect("not a DLL", junk, -1);
    std::vector<uint8_t> cut = make_dll(both);
    cut.resize(cut.size() / 2);
    expect("cut short", cut, -1);
    printf("%s\n", fails ? "lsfg dll check: FAILED" : "lsfg dll check: all passed");
    return fails ? 1 : 0;
}
