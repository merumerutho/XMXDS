/*
 * cueScreen.c
 */
#include <stdio.h>
#include "cueScreen.h"
#include "arm9_defines.h"
#include "arm9_fifo.h"
#include "screens.h"

/* ------------------------------------------------------------------ */
/* Layout constants                                                     */
/* ------------------------------------------------------------------ */

/* Cue grid occupies rows 1-6 (2 blocks of 3 rows: sep + label + pos) */
#define CUE_GRID_TOP_ROW    1
#define CUE_GRID_BOT_ROW    7   /* separator below second cue row */
#define CUE_BLOCK_H         3   /* sep + label + position */

#define CUE_GRID_TOP_FRAC   ((float)CUE_GRID_TOP_ROW / 24)
#define CUE_GRID_BOT_FRAC   ((float)CUE_GRID_BOT_ROW / 24)
#define TAB_FRAC            (1.0f / 24)

/* ------------------------------------------------------------------ */
/* Tap tempo state                                                       */
/* ------------------------------------------------------------------ */

#define TAP_BUFFER_SIZE     4
#define TAP_TIMEOUT_FRAMES  120     /* 2 s at ~60 Hz — resets buffer */

static u32 s_tapIntervals[TAP_BUFFER_SIZE];
static u8  s_tapCount       = 0;
static u32 s_frameCounter   = 0;
static u32 s_lastTapFrame   = 0;

/* ------------------------------------------------------------------ */

void cueScreen_tick(void)
{
    s_frameCounter++;
    if (s_tapCount > 0 &&
        (s_frameCounter - s_lastTapFrame) > TAP_TIMEOUT_FRAMES)
        s_tapCount = 0;
}

/* ------------------------------------------------------------------ */

void drawCueCell(u8 idx)
{
    consoleSelect(&bottom);
    u8 block  = idx / 4;           /* 0 = top row, 1 = bottom row */
    u8 col    = (idx % 4) * 8 + 1;
    u8 row    = CUE_GRID_TOP_ROW + block * CUE_BLOCK_H + 1; /* label row */

    iprintf("\x1b[%d;%dHCUE%-2d  ", row,     col, idx + 1);
    iprintf("\x1b[%d;%dHP:%-3d   ", row + 1, col, arm9_cuePoints[idx] + 1);
}

/* ------------------------------------------------------------------ */

void drawCueScreen(void)
{
    consoleSelect(&bottom);
    consoleClear();

    drawTabStrip(SCREEN_MODE_CUE);

    /* Vertical separators for cue grid (rows 1-6) */
    for (u8 r = CUE_GRID_TOP_ROW; r < CUE_GRID_BOT_ROW; r++) {
        iprintf("\x1b[%d;0H|",  r);
        iprintf("\x1b[%d;8H|",  r);
        iprintf("\x1b[%d;16H|", r);
        iprintf("\x1b[%d;24H|", r);
    }

    /* Horizontal separators: top of each cue block + bottom edge */
    for (u8 block = 0; block <= 2; block++) {
        u8 row = CUE_GRID_TOP_ROW + block * CUE_BLOCK_H;
        for (u8 c = 0; c < 32; c++)
            iprintf("\x1b[%d;%dH-", row, c);
    }

    /* Cue cells */
    for (u8 i = 0; i < N_CUES; i++)
        drawCueCell(i);

    /* Tap tempo zone */
    iprintf("\x1b[10;8H TAP  TEMPO ");
    iprintf("\x1b[12;8H BPM: %3d   ", arm9_globalBpm);
    iprintf("\x1b[14;6H  (tap the screen)  ");
}

/* ------------------------------------------------------------------ */

static void refreshTapBpm(void)
{
    consoleSelect(&bottom);
    iprintf("\x1b[12;8H BPM: %3d   ", arm9_globalBpm);
}

/* ------------------------------------------------------------------ */

static void registerTap(void)
{
    if (s_tapCount > 0) {
        u32 interval = s_frameCounter - s_lastTapFrame;
        s_tapIntervals[(s_tapCount - 1) % TAP_BUFFER_SIZE] = interval;
    }
    s_lastTapFrame = s_frameCounter;
    if (s_tapCount <= TAP_BUFFER_SIZE)
        s_tapCount++;

    /* Need at least one stored interval (= 2 taps) to compute BPM */
    u8 n = (s_tapCount - 1 < TAP_BUFFER_SIZE) ? s_tapCount - 1 : TAP_BUFFER_SIZE;
    if (n < 1) return;

    u32 sum = 0;
    for (u8 i = 0; i < n; i++) sum += s_tapIntervals[i];
    u32 avg = sum / n;
    if (avg == 0) return;

    /* BPM = 60 s * 60 fps / avg_frames */
    u32 bpm = 3600u / avg;
    if (bpm < 20)  bpm = 20;
    if (bpm > 255) bpm = 255;

    arm9_globalBpm = (u8)bpm;
    serviceUpdate(0);
    refreshTapBpm();
}

/* ------------------------------------------------------------------ */

void handleCueTouch(touchPosition *touchPos, bool b_held, u8 current_song_pos)
{
    float x = (touchPos->rawx - X_MIN) * X_NORM;
    float y = (touchPos->rawy - Y_MIN) * Y_NORM;

    /* Tab strip is handled by main.c before calling here */
    if (y < TAB_FRAC) return;

    /* Cue grid zone */
    if (y < CUE_GRID_BOT_FRAC) {
        float y_in_cues = (y - CUE_GRID_TOP_FRAC) /
                          (CUE_GRID_BOT_FRAC - CUE_GRID_TOP_FRAC);
        u8 cue_row = (u8)(y_in_cues * 2);
        u8 cue_col = (u8)(x * 4);
        if (cue_row > 1) cue_row = 1;
        if (cue_col > 3) cue_col = 3;
        u8 idx = cue_col + cue_row * 4;

        if (b_held) {
            arm9_cuePoints[idx] = current_song_pos;
            drawCueCell(idx);
        } else {
            serviceCmd(CMD_GOTO_HOTCUE, arm9_cuePoints[idx]);
        }
        return;
    }

    /* Tap tempo zone (everything below the cue grid) */
    registerTap();
}
