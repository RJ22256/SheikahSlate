// WSword: the Glass Slate look - the start screen's panel and card, Settings' rail of pages, the menus' navy glass and
// Inter fonts - and the options made on the Galaxy Tab S10 Ultra (Video > Mode, Even frame pacing, Frame generation),
// on the Android app and on the PC build (Windows). A macOS / Linux build of the shared desktop code keeps LAS
// Recomp's look.
#pragma once
#if defined(__ANDROID__) || defined(_WIN32)
#define WS_SLATE_UI 1
#endif
#ifndef WSWORD_VERSION
#define WSWORD_VERSION ""  // (the PC build's: the Makefile passes the Android app's versionName)
#endif
