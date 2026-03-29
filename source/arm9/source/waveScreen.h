/*
 * waveScreen.h
 *
 * Bottom screen WAVE mode: 4x4 grid of per-channel sample waveforms rendered
 * on BG2 (8bpp bitmap), with the text console (BG0) overlaid for the tab strip.
 *
 * Grid layout (pixel coordinates, BG2):
 *
 *   y=0..7   : behind tab strip text (BG0) — bitmap pixels visible but obscured
 *   y=8      : top grid border
 *   y=8..191 : 4 rows x 46px each  (border + 44px inner)
 *   x=0..255 : 4 cols x 64px each  (border + 62px inner)
 *
 * Touch zones per cell:
 *   left  half (x < 32) -> mute toggle
 *   right half (x >= 32) -> solo toggle
 *
 * No ARM7 changes required: sample data is read directly from the module struct
 * and instrument/sample pointers already in main RAM.
 */
#ifndef ARM9_SOURCE_WAVESCREEN_H_
#define ARM9_SOURCE_WAVESCREEN_H_

#include <nds.h>

/* Full redraw of the wave screen.
   Clears the bitmap, draws grid borders, redraws all 16 cells.
   BG2 show/hide is handled by the caller (redrawBottomScreen). */
void drawWaveScreen(void);

/* Called once per VBlank frame.
   Detects note/instrument/volume changes per channel and redraws dirty cells. */
void waveScreen_tick(void);

/* Handle a touch on the wave screen.
   Left half of a cell = mute toggle; right half = solo toggle.
   Unused channels (idx >= module channel count) are ignored.
   Tab strip touch is handled by main.c before calling here. */
void handleWaveTouch(touchPosition *touchPos);

#endif /* ARM9_SOURCE_WAVESCREEN_H_ */
