#!/bin/sh
# WSword: builds third_party/lsfg-vk (the frame generation backend) on the host and runs tools/test_lsfg_dll.cpp -
# the Lossless.dll check against made-up DLLs. Vulkan headers: $1 (an include folder with vulkan/vulkan_core.h),
# else the system's.
set -e
cd "$(dirname "$0")/.."
L=third_party/lsfg-vk
VK=${1:+-I$1}
mkdir -p build/lsfg
objs=""
for f in $(find $L/lsfg-vk-common/src $L/lsfg-vk-backend/src -name '*.cpp' | sort); do
    o=build/lsfg/$(echo "$f" | tr / _).o
    clang++ -std=c++20 -O1 -c -DVK_NO_PROTOTYPES -I$L/lsfg-vk-common/include -I$L/lsfg-vk-backend/include \
        -I$L/lsfg-vk-backend/src $VK "$f" -o "$o"
    objs="$objs $o"
done
clang++ -std=c++20 -O1 -DVK_NO_PROTOTYPES -I$L/lsfg-vk-backend/include $VK tools/test_lsfg_dll.cpp $objs -ldl \
    -o build/test_lsfg_dll
build/test_lsfg_dll
