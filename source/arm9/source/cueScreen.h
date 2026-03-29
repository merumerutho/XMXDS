/*
 * cueScreen.h
 *
 * Bottom screen layout (CUE+TAP mode):
 *
 *   Row  0 : tab strip
 *   Row  1 : ---- separator ----
 *   Row  2 : CUE1  CUE2  CUE3  CUE4   <- label + position
 *   Row  3 : pos   pos   pos   pos
 *   Row  4 : ---- separator ----
 *   Row  5 : CUE5  CUE6  CUE7  CUE8
 *   Row  6 : pos   pos   pos   pos
 *   Row  7 : ---- separator ----
 *   Rows 8-23: tap tempo zone
 */
#ifndef ARM9_SOURCE_CUESCREEN_H_
#define ARM9_SOURCE_CUESCREEN_H_

#include <nds.h>

/* Must be called once per VBlank frame from the main loop. */
void cueScreen_tick(void);

/* Full redraw of the cue screen (clears bottom screen). */
void drawCueScreen(void);

/* Incremental update of one cue cell. */
void drawCueCell(u8 idx);

/* Handle a touch on the cue screen.
   b_held: true when KEY_B is currently held (SET cue instead of JUMP).
   current_song_pos: MODULE->CurrentSongPosition passed in by caller. */
void handleCueTouch(touchPosition *touchPos, bool b_held, u8 current_song_pos);

#endif /* ARM9_SOURCE_CUESCREEN_H_ */
