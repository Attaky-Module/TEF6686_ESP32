#ifndef CONFIG_H
#define CONFIG_H

// Restored file. Upstream deleted src/config.h in aed3b95 ("Removed config.h",
// 2025-03-19 12:49) but left the three `#include "config.h"` lines in
// constants.h, gui.cpp and touch.cpp, and tagged v2.20 (e813a3a) 69 minutes
// later — so the v2.20 release does not compile as published. The content
// below is upstream's own last version of the file (aed3b95^), unchanged.
//
// This is where HAS_AIR_BAND is decided. TEF6686_ESP32.ino carries its own
// commented-out `#define HAS_AIR_BAND`, but the .ino includes constants.h
// BEFORE that line, so the .ino copy can never reach constants.h — only this
// header can. Set the build option here, not there.

#include <Arduino.h>

// #define HAS_AIR_BAND        // uncomment to enable Air Band(Make sure you have Air Band extend board)
// #define DEEPELEC_DP_66X     // uncomment for DEEPELEC Portable DP-66X build (Simplified Chinese)

#endif
