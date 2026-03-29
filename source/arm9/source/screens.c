/*
 * screens.c
 *
 */
#include <stdio.h>
#include "screens.h"

PrintConsole top, bottom;

const float X_NORM = (float) 1 / (X_MAX - X_MIN);
const float Y_NORM = (float) 1 / (Y_MAX - Y_MIN);

int sub_bg2 = -1;

/* Tab strip: 2 tabs across 32 chars — 16 + 16 */
void drawTabStrip(ScreenMode mode)
{
    consoleSelect(&bottom);
    iprintf("\x1b[0;0H%-16s%-16s",
        mode == SCREEN_MODE_CH  ? ">CH"      : " CH",
        mode == SCREEN_MODE_CUE ? ">CUE+TAP" : " CUE+TAP");
}

void initWaveBg(void)
{
    /* BG2 as 256x256 8bpp bitmap.
     * mapBase=4 places bitmap data at 4x16KB = 64KB into VRAM_C,
     * safely above the text console's tile (0KB) and map (4KB) regions. */
    sub_bg2 = bgInitSub(2, BgType_Bmp8, BgSize_B8_256x256, 4, 0);

    /* Draw BG2 behind BG0 (text console).
     * Lower priority number = higher z-order; BG0 stays on top at priority 0. */
    bgSetPriority(sub_bg2, 2);

    /* Waveform colour palette (shared with text console's BG_PALETTE_SUB).
     * Index 0 doubles as the text-transparency colour, so it stays black. */
    BG_PALETTE_SUB[0] = RGB15( 0,  0,  0);  /* black       — cell background        */
    BG_PALETTE_SUB[1] = RGB15( 0, 12,  0);  /* dim green   — borders / centre line  */
    BG_PALETTE_SUB[2] = RGB15( 0, 31,  0);  /* bright green — normal waveform       */
    BG_PALETTE_SUB[3] = RGB15( 0,  6,  0);  /* very dim green — muted waveform      */
    BG_PALETTE_SUB[4] = RGB15(31, 31,  0);  /* bright yellow  — soloed waveform     */

    /* Keep hidden until WAVE mode is first entered */
    bgHide(sub_bg2);
}
