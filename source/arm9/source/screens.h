/*
 * screens.h
 *
 *  Created on: 19 gen 2023
 *      Author: merut
 */

#ifndef ARM9_SOURCE_SCREENS_H_
#define ARM9_SOURCE_SCREENS_H_

#include <nds.h>
#include "arm9_defines.h"

#define X_MIN 320
#define X_MAX 3808
#define Y_MIN 224
#define Y_MAX 3904

extern const float X_NORM;
extern const float Y_NORM;

extern PrintConsole top, bottom;

/* libnds BG handle for the sub-screen bitmap layer (BG2).
   Initialised by initWaveBg(); -1 until then. */
extern int sub_bg2;

/* Draw the three-tab strip on row 0 of the bottom screen,
   highlighting the currently active mode. */
void drawTabStrip(ScreenMode mode);

/* One-time setup: initialise BG2 as a 256x256 8bpp bitmap behind the text
   console (BG0), configure the waveform colour palette, and hide BG2.
   Must be called after consoleInit for the sub screen. */
void initWaveBg(void);

#endif /* ARM9_SOURCE_SCREENS_H_ */
