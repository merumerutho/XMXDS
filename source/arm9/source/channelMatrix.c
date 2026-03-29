/*
 * channelMatrix.c
 *
 * Bottom screen layout (CH mode):
 *
 *   Row  0 : tab strip  (drawn by drawTabStrip)
 *   Row  1 : ---- separator ----
 *   Row  2 : Ch.1  Ch.2  Ch.3  Ch.4    <- channel label
 *   Row  3 : mute status row
 *   Row  4 : solo status row
 *   Row  5 : ---- separator ----
 *   ...repeats for rows 5-8, 9-12, 13-16...
 *   Row 17 : ---- separator ----  (bottom edge)
 *
 * Touch zones per cell (y within the 4-row content block, excluding sep):
 *   upper half  -> mute toggle
 *   lower half  -> solo toggle
 */
#include <nds.h>
#include <stdio.h>

#include "channelMatrix.h"
#include "screens.h"
#include "arm9_fifo.h"
#include "libXMX.h"

#define MODULE  (deckInfo.modManager)

/* Row layout constants */
#define CH_BLOCK_H      4       /* rows per channel block (sep + label + mute + solo) */
#define CH_GRID_OFFSET  1       /* first row used by the grid (row 0 = tab strip) */
#define CH_GRID_BLOCKS  4       /* number of channel rows (4 x 4 channels = 16) */

/* Row indices for a given block i (0..3) */
#define CH_ROW_SEP(i)   (CH_GRID_OFFSET + (i) * CH_BLOCK_H)
#define CH_ROW_LABEL(i) (CH_GRID_OFFSET + (i) * CH_BLOCK_H + 1)
#define CH_ROW_MUTE(i)  (CH_GRID_OFFSET + (i) * CH_BLOCK_H + 2)
#define CH_ROW_SOLO(i)  (CH_GRID_OFFSET + (i) * CH_BLOCK_H + 3)
#define CH_ROW_BOTTOM   (CH_GRID_OFFSET + CH_GRID_BLOCKS * CH_BLOCK_H)  /* row 17 */

/* Normalised y boundaries */
#define TAB_FRAC        (1.0f / 24)
#define GRID_TOP_FRAC   TAB_FRAC
#define GRID_BOT_FRAC   ((float)(CH_ROW_BOTTOM) / 24)

/* ------------------------------------------------------------------ */

void drawChannelCell(u8 idx)
{
    consoleSelect(&bottom);
    u8 block = idx / 4;
    u8 col   = (idx % 4) * 8 + 1;

    if (MODULE == NULL || idx >= MODULE->NumberofChannels) {
        iprintf("\x1b[%d;%dH%-7s", CH_ROW_MUTE(block), col, "-----");
        iprintf("\x1b[%d;%dH%-7s", CH_ROW_SOLO(block), col, "     ");
        return;
    }

    iprintf("\x1b[%d;%dH%-7s", CH_ROW_MUTE(block), col,
            arm9_channelMute[idx] ? "Muted" : "     ");
    iprintf("\x1b[%d;%dH%-7s", CH_ROW_SOLO(block), col,
            arm9_soloChannel == (s8)idx ? "Solo " : "     ");
}

/* ------------------------------------------------------------------ */

void drawChannelMatrix(void)
{
    consoleSelect(&bottom);
    consoleClear();

    drawTabStrip(SCREEN_MODE_CH);

    /* Vertical separators (cols 0, 8, 16, 24), rows 1-17 */
    for (u8 r = CH_GRID_OFFSET; r <= CH_ROW_BOTTOM; r++) {
        iprintf("\x1b[%d;0H|",  r);
        iprintf("\x1b[%d;8H|",  r);
        iprintf("\x1b[%d;16H|", r);
        iprintf("\x1b[%d;24H|", r);
    }

    /* Horizontal separators at top of each block and at the bottom edge */
    for (u8 i = 0; i <= CH_GRID_BLOCKS; i++) {
        u8 row = CH_GRID_OFFSET + i * CH_BLOCK_H;
        for (u8 c = 0; c < 32; c++)
            iprintf("\x1b[%d;%dH-", row, c);
    }

    /* Channel labels */
    for (u8 i = 0; i < CH_GRID_BLOCKS; i++)
        for (u8 j = 0; j < 4; j++)
            iprintf("\x1b[%d;%dHCh.%-2d", CH_ROW_LABEL(i), j * 8 + 2, i * 4 + j + 1);

    /* Cell statuses */
    for (u8 i = 0; i < 16; i++)
        drawChannelCell(i);
}

/* ------------------------------------------------------------------ */

static void handleChannelSolo(u8 idx)
{
    if (arm9_soloChannel == (s8)idx) {
        /* Un-solo: restore pre-solo mutes */
        arm9_soloChannel = -1;
        for (u8 i = 0; i < 16; i++) {
            arm9_channelMute[i] = arm9_preSoloMute[i];
            serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)i << 8) | arm9_channelMute[i]);
        }
    } else {
        /* Save current mutes only if entering solo fresh (not switching solo target) */
        if (arm9_soloChannel < 0) {
            for (u8 i = 0; i < 16; i++)
                arm9_preSoloMute[i] = arm9_channelMute[i];
        }
        arm9_soloChannel = (s8)idx;
        for (u8 i = 0; i < 16; i++) {
            arm9_channelMute[i] = (i != idx) ? 1 : 0;
            serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)i << 8) | arm9_channelMute[i]);
        }
    }

    for (u8 i = 0; i < 16; i++)
        drawChannelCell(i);
}

/* ------------------------------------------------------------------ */

void handleChannelTouch(touchPosition *touchPos)
{
    float x = (touchPos->rawx - X_MIN) * X_NORM;
    float y = (touchPos->rawy - Y_MIN) * Y_NORM;

    /* Tab strip touch is handled by main.c before calling here */
    if (y < GRID_TOP_FRAC || y >= GRID_BOT_FRAC) return;

    float y_grid    = (y - GRID_TOP_FRAC) / (GRID_BOT_FRAC - GRID_TOP_FRAC);
    u8    block_i   = (u8)(y_grid * CH_GRID_BLOCKS);
    if (block_i >= CH_GRID_BLOCKS) block_i = CH_GRID_BLOCKS - 1;
    float y_in_block = y_grid * CH_GRID_BLOCKS - block_i;

    u8 col = (u8)(x * 4);
    if (col > 3) col = 3;

    u8 idx = col + block_i * 4;

    /* Ignore unused channels */
    if (MODULE == NULL || idx >= MODULE->NumberofChannels) return;

    if (y_in_block < 0.5f) {
        /* Upper half: mute toggle */
        arm9_channelMute[idx] ^= 1;
        serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)idx << 8) | arm9_channelMute[idx]);
        drawChannelCell(idx);
    } else {
        /* Lower half: solo toggle */
        handleChannelSolo(idx);
    }
}
