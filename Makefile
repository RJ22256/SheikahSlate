# WSword for Windows 10/11 x64 - the PC build (from LAS Recomp's Makefile, MIT; the Android app is android/, Gradle).
#   On Windows: an MSYS2 UCRT64 shell with clang (docs/BUILD-windows.md): make, make dist.
#   From Linux (compile checks, CI): llvm-mingw - make WINCROSS=<llvm-mingw>/bin/x86_64-w64-mingw32- DEPS=<prefix>.
# The game code isn't in what this builds: the start screen builds it on the player's PC from their own game files
# (runtime/gamebuild.c), with the generator helper (botwgen/, make gen-helper) and the bundled compiler (zig/).
comma := ,
B        ?= build/win
DEPS     ?=
WINCROSS ?=
CC       := $(WINCROSS)clang
CXX      := $(WINCROSS)clang++
AR       := $(WINCROSS)ar  # (MSYS2: binutils' ar - clang's COFF objects)
WINDRES  ?= $(if $(WINCROSS),$(WINCROSS)windres,windres)
NO_FFMPEG ?=
IMGUI    := third_party/imgui
LSFG     := third_party/lsfg-vk
EXE      := $(B)/WSword.exe
IMPLIB   := $(B)/WSword.dll.a

# The game code's compile flags (the start screen's: runtime/gamebuild.c) - AVX2 + FMA for the lifted FP/SIMD code
# (runtime/simd.h, portable C on x86-64; checked at startup). Part of its id: another set builds it again.
GENARCH  ?= -march=x86-64-v3
GENFLAGS := -O1 -fno-strict-aliasing -Wno-return-type -Wno-unused-label -fvisibility=hidden -D_FILE_OFFSET_BITS=64 $(GENARCH)
GAMELIB_SHARED := -shared
GAMELIB_SRCS := tools/gen.py tools/lifter.py tools/analyze.py tools/nso.py tools/passes.py tools/patches.py tools/simd_c.py \
                tools/gamelib_entry.c runtime/hle/hle_symbols.txt runtime/cpu.h runtime/simd.h runtime/simd_fp.h runtime/gamelib.h
GAMELIB_ID := $(shell (cat $(GAMELIB_SRCS); echo '$(GENFLAGS) --simd c') | (sha256sum 2>/dev/null || shasum -a 256) | cut -c1-16)

# the PC build's version (the start screen's "App" and the zip's name): the Android app's it was made from, then pc.<n>
VERSION  := 0.2.8-pc.3
OSFLAGS  := -D_WIN32_WINNT=0x0A00 -D_FILE_OFFSET_BITS=64 -D_POSIX_THREAD_SAFE_FUNCTIONS -D_CRT_RAND_S \
            -DWIN32_LEAN_AND_MEAN -DNOMINMAX $(if $(RELEASE),-DLARECOMP_ZIP_BUILD) $(if $(NO_FFMPEG),-DLARECOMP_NO_FFMPEG)
DEPINC   := $(if $(DEPS),-I$(DEPS)/include)
PATHMAP  := -ffile-prefix-map=$(CURDIR)=.
RTFLAGS  := -O2 -g -fno-strict-aliasing -Wall -Wno-unused-function -Wno-format -Iruntime -Iruntime/nvn $(DEPINC) $(OSFLAGS) \
            $(PATHMAP) -I$(LSFG)/lsfg-vk-backend/include
CXXFLAGS := -O2 -g -std=c++17 -fno-strict-aliasing -I$(IMGUI) -I$(IMGUI)/backends -Iruntime -Iruntime/nvn $(DEPINC) \
            $(OSFLAGS) $(PATHMAP) -I$(LSFG)/lsfg-vk-backend/include
FFMPEG_LIBS := $(if $(NO_FFMPEG),,-lavformat -lavcodec -lavutil -lswscale)
LDFLAGS  := -fuse-ld=lld -Wl,--stack,16777216 $(if $(DEPS),-L$(DEPS)/lib) -lSDL3 $(FFMPEG_LIBS) -lvulkan-1 -lshaderc_shared \
            -lws2_32 -lwinmm -lole32 -lshell32 -lcomdlg32 -luuid -ldbghelp -lpsapi -lpowrprof -pthread \
            -Wl,--export-all-symbols -Wl,--out-implib,$(IMPLIB) \
            $(if $(RELEASE),-Wl$(comma)--subsystem$(comma)windows -Wl$(comma)--strip-debug)
# (release: no console window - the log goes to WSword.log next to the exe, fatal errors show a message box)

RT_SRC   := $(filter-out $(if $(NO_FFMPEG),runtime/hle/movie.c),$(wildcard runtime/*.c) $(wildcard runtime/hle/*.c) \
            $(wildcard runtime/services/*.c) $(wildcard runtime/nvn/*.c) $(wildcard runtime/nvn/maxwell/*.c)) \
            runtime/platform/windows.c
BACKEND_SRC := $(filter-out runtime/nvn/vulkan/android_driver.c,$(wildcard runtime/nvn/vulkan/*.c) $(wildcard runtime/nvn/vulkan/*.cpp))
UI_SRC   := $(wildcard runtime/ui/*.mm)
RT_HDRS  := $(wildcard runtime/*.h runtime/platform/*.h runtime/nvn/*.h runtime/nvn/maxwell/*.h runtime/nvn/vulkan/*.h \
              runtime/ui/*.h runtime/ui/*.inc)
RT_OBJ   := $(patsubst runtime/%.c,$(B)/rt/%.o,$(RT_SRC))
BACKEND_OBJ := $(patsubst runtime/%.cpp,$(B)/rt/%.o,$(patsubst runtime/%.c,$(B)/rt/%.o,$(BACKEND_SRC))) $(B)/rt/platform/windows_rc.o
UI_OBJ   := $(patsubst runtime/%.mm,$(B)/rt/%.o,$(UI_SRC))
IMGUI_OBJ := $(B)/imgui/imgui.o $(B)/imgui/imgui_draw.o $(B)/imgui/imgui_tables.o $(B)/imgui/imgui_widgets.o \
             $(B)/imgui/imgui_impl_sdl3.o $(B)/imgui/imgui_impl_vulkan.o
# frame generation (runtime/nvn/vulkan/framegen.cpp): lsfg-vk's backend, GPL-3.0 (third_party/lsfg-vk/README.WSword.md)
LSFG_SRC := $(shell find $(LSFG)/lsfg-vk-common/src $(LSFG)/lsfg-vk-backend/src -name '*.cpp' | sort)
LSFG_OBJ := $(patsubst $(LSFG)/%.cpp,$(B)/lsfg/%.o,$(LSFG_SRC))
LSFG_LIB := $(B)/liblsfgvk.a
LSFGFLAGS := -O2 -g -std=c++20 -fexceptions -frtti -DVK_NO_PROTOTYPES -Wno-unknown-warning-option $(DEPINC) $(OSFLAGS) \
             -I$(LSFG)/lsfg-vk-common/include -I$(LSFG)/lsfg-vk-backend/include -I$(LSFG)/lsfg-vk-backend/src $(PATHMAP)

all: $(EXE)

# linked to a folder of its own, then moved: a copy still running keeps its file. The import library records the name
# the game code library loads the exe by (WSword.exe).
$(EXE): $(RT_OBJ) $(BACKEND_OBJ) $(UI_OBJ) $(IMGUI_OBJ) $(LSFG_LIB)
	@mkdir -p $(dir $@)link
	$(CXX) -o $(dir $@)link/$(notdir $@) $(RT_OBJ) $(BACKEND_OBJ) $(UI_OBJ) $(IMGUI_OBJ) $(LSFG_LIB) \
	    $(subst $(IMPLIB),$(dir $@)link/WSword.dll.a,$(LDFLAGS))
	mv -f $(dir $@)link/$(notdir $@) $@
	mv -f $(dir $@)link/WSword.dll.a $(IMPLIB)

$(B)/rt/gamelib.o: $(GAMELIB_SRCS)
$(B)/rt/gamelib.o: RTFLAGS += -DGAMELIB_ID='"$(GAMELIB_ID)"'
# the start screen's build of the game code uses the same flags (runtime/gamebuild.c)
$(B)/rt/gamebuild.o: $(GAMELIB_SRCS) Makefile
$(B)/rt/gamebuild.o: RTFLAGS += -DGAMELIB_ID='"$(GAMELIB_ID)"' -DGAMELIB_CFLAGS='"$(GENFLAGS)"' \
                                -DGAMELIB_SHARED='"$(GAMELIB_SHARED)"'

$(B)/rt/%.o: runtime/%.c $(RT_HDRS)
	@mkdir -p $(dir $@)
	$(CC) -std=gnu17 $(RTFLAGS) -c $< -o $@

$(B)/rt/%.o: runtime/%.cpp $(RT_HDRS)
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# the UI sources are plain C++ apart from __APPLE__ blocks
$(B)/rt/ui/%.o: runtime/ui/%.mm $(RT_HDRS)
	@mkdir -p $(dir $@)
	$(CXX) -x c++ $(CXXFLAGS) -DWSWORD_VERSION='"$(VERSION)"' -c $< -o $@

# SMAA (third_party/smaa, MIT): the reference shader as a C string for runtime/nvn/smaa_data.c; non-ASCII bytes in its
# comments as '?', CRLF as LF (as the Android build's Gradle task does)
$(B)/inc/smaa_hlsl.inc: third_party/smaa/SMAA.hlsl
	@mkdir -p $(dir $@)
	LC_ALL=C tr -d '\r' < $< | LC_ALL=C tr '\200-\377' '?' | sed -e 's/\\/\\\\/g' -e 's/"/\\"/g' -e 's/^/"/' -e 's/$$/\\n"/' > $@
$(B)/rt/nvn/smaa_data.o: $(B)/inc/smaa_hlsl.inc
$(B)/rt/nvn/smaa_data.o: RTFLAGS += -I$(B)/inc -Ithird_party/smaa/Textures

$(B)/rt/platform/windows_rc.o: runtime/platform/windows.rc docs/icon/wsword.ico runtime/platform/windows.manifest
	@mkdir -p $(dir $@)
	$(WINDRES) -I docs/icon -I runtime/platform -O coff $< -o $@

$(B)/imgui/%.o: $(IMGUI)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@
$(B)/imgui/imgui_impl_%.o: $(IMGUI)/backends/imgui_impl_%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(B)/lsfg/%.o: $(LSFG)/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(LSFGFLAGS) -c $< -o $@
$(LSFG_LIB): $(LSFG_OBJ)
	rm -f $@
	$(AR) rcs $@ $^

# Runtime objects only (no link): a quick compile check
objs: $(RT_OBJ) $(BACKEND_OBJ) $(UI_OBJ) $(IMGUI_OBJ) $(LSFG_LIB)

# build/dist/WSword-<version>-windows.zip, top folder "WSword/": the exe and its import library, the generator helper
# (botwgen/), the compiler for the player's PC (zig/: tools/fetch_zig.sh, slimmed by tools/zig_slim.sh), the headers
# the game code is compiled against (gamelib-src/), the fonts and art the start screen shows, and the DLLs they load
# (runs without MSYS2; vulkan-1.dll is the GPU driver's, in System32). No game code, no game files, no Lossless.dll.
# Needs: Windows 10+ x64, a Vulkan 1.3 driver, an AVX2 CPU.
DIST     := build/dist/WSword
WIN_B    := build/win-release
# the LGPL FFmpeg the release links (MSYS2's is a GPLv3 build with many codec libraries): tools/build_deps_windows.sh
deps-windows:
	tools/build_deps_windows.sh
dist:
	@[ -f build/deps-win/lib/libavcodec.dll.a ] || { echo "make deps-windows first (FFmpeg for the game's movies)"; exit 1; }
	@[ -d build/helper/botwgen ] || { echo "make gen-helper first (the game code generator)"; exit 1; }
	@[ -x build/zig/zig.exe ] || { echo "tools/fetch_zig.sh first (the compiler)"; exit 1; }
	$(MAKE) B=$(WIN_B) DEPS=$(CURDIR)/build/deps-win RELEASE=1 $(WIN_B)/WSword.exe
	! nm $(WIN_B)/WSword.exe | grep ' f_70' >/dev/null  # (no game code inside)
	rm -rf build/dist && mkdir -p "$(DIST)/gamelib-src"
	cp $(WIN_B)/WSword.exe $(WIN_B)/WSword.dll.a "$(DIST)/"
	IMPLIB=$(WIN_B)/WSword.dll.a tools/zig_slim.sh $(GENFLAGS)
	cp -R build/helper/botwgen "$(DIST)/"
	cp -R build/zig-slim "$(DIST)/zig"
	cp runtime/cpu.h runtime/simd.h runtime/simd_fp.h runtime/gamelib.h tools/gamelib_entry.c runtime/hle/hle_symbols.txt \
	    "$(DIST)/gamelib-src/"
	tools/win_resources.sh "$(DIST)"
	cp README.md LICENSE third_party/lsfg-vk/LICENSE.md "$(DIST)/"
	cp docs/PC.md "$(DIST)/WSword-PC-guide.md"
	cp tools/third_party_windows.txt "$(DIST)/THIRD-PARTY.txt"
	cp third_party/lsfg-vk/README.WSword.md "$(DIST)/lsfg-vk-README.md"
	PATH="$(CURDIR)/build/deps-win/bin:$$PATH" ldd $(WIN_B)/WSword.exe | awk '(/ucrt64/ || /deps-win/) && !/vulkan-1/ {print $$3}' | \
	    sort -u | xargs -I{} cp -u {} "$(DIST)/"
	cd build/dist && if command -v zip >/dev/null; then zip -qr "WSword-$(VERSION)-windows.zip" WSword; \
	    else /c/Windows/System32/tar.exe -a -cf "WSword-$(VERSION)-windows.zip" WSword; fi
	@echo "build/dist/WSword-$(VERSION)-windows.zip"

# The game code generator as a standalone helper, so players need no Python: build/helper/botwgen/ (botwgen.exe +
# _internal/), frozen by PyInstaller from tools/botwgen.spec in its own venv. PyInstaller builds for the OS it runs on.
GEN_PY  ?= python
GEN_VPY := build/gen-venv/Scripts/python.exe
gen-helper: build/gen-venv/.ok
	$(GEN_VPY) -m PyInstaller --noconfirm --clean --log-level WARN --distpath build/helper --workpath build/pyinstaller \
	    tools/botwgen.spec
	$(GEN_VPY) tools/pe_no_cfg.py build/helper/botwgen/botwgen.exe
	@du -sh build/helper/botwgen
build/gen-venv/.ok:
	$(GEN_PY) -m venv build/gen-venv
	$(GEN_VPY) -m pip install -q pyinstaller==6.22.3 capstone==5.0.9 unicorn==2.1.4 lz4==4.4.5
	touch $@

.PHONY: all objs dist gen-helper deps-windows
