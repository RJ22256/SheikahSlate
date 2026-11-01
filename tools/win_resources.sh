#!/bin/sh
# usage: tools/win_resources.sh <folder>: what WSword.exe's start screen and menus load from next to it
# (runtime/paths.c rt_resource) - the art and the fonts (OFL), under the names the Android app's assets use.
set -eu
cd "$(dirname "$0")/.."
D=$1
mkdir -p "$D"
cp assets/artwork/great-plateau-background-v2.png "$D/launcher-background.png"
cp assets/artwork/app-icons/app-icon-sword.png "$D/icon.png"
for n in slate midnight dawn ancient forest; do cp "assets/artwork/app-icons/app-icon-$n.png" "$D/"; done
cp third_party/fonts/Baloo2/Baloo2-SemiBold.ttf third_party/fonts/Inter/Inter-Bold.otf third_party/fonts/Inter/Inter-Medium.otf "$D/"
cp third_party/fonts/Baloo2/OFL.txt "$D/Baloo2-OFL.txt"
cp third_party/fonts/Inter/OFL.txt "$D/Inter-OFL.txt"
