/*
 * waveScreen.c
 *
 * Bottom screen CH mode: 4x4 waveform grid with per-channel mute/solo controls.
 *
 * BG2 (8bpp bitmap) renders the waveforms; BG0 (text console) overlays channel
 * labels and mute/solo status text on top.
 *
 * Sample data is read directly from the MODULE struct and the instrument/sample
 * pointers already in main RAM — no ARM7 changes or new IPC required.
 *
 * Cache notes:
 *   MODULE struct  : DC_InvalidateRange'd each frame by drawTitle() — fresh on tick.
 *   Instrument/sample structs + sample data : loaded by ARM9, only read by ARM7.
 *     ARM9 cache is authoritative; no extra invalidation needed.
 *   VRAM (BG2 bitmap) : uncacheable on ARM9 — writes are immediately visible to GPU.
 *
 * Touch zones per cell:
 *   left  half (x < 32) -> mute toggle
 *   right half (x >= 32) -> solo toggle
 *
 * Waveform colour:
 *   bright green  — normal channel
 *   very dim green — muted channel
 *   bright yellow  — soloed channel
 *
 * Amplitude:
 *   Waveform height is scaled by MODULE->CurrentSampleVolume[ch] (0..0x40) so
 *   the display pulses with the volume envelope in real time.
 */
#include <stdio.h>
#include <string.h>
#include "waveScreen.h"
#include "screens.h"
#include "arm9_defines.h"
#include "arm9_fifo.h"    /* arm9_channelMute, arm9_soloChannel, arm9_preSoloMute, serviceCmd */
#include "libXMX.h"
#include "libxm7.h"

#define MODULE (deckInfo.modManager)

/* ------------------------------------------------------------------ */
/* Grid geometry                                                         */
/* ------------------------------------------------------------------ */

#define WAVE_GRID_TOP   8   /* first pixel row of the grid              */
#define WAVE_CELL_W    64   /* pixels per column (including left border) */
#define WAVE_CELL_H    46   /* pixels per row    (including top border)  */
#define WAVE_INNER_W   62   /* usable waveform width  (cell - 2 borders) */
#define WAVE_INNER_H   44   /* usable waveform height (cell - 2 borders) */

#define WAVE_INNER_X(c)  ((c) * WAVE_CELL_W + 1)
#define WAVE_INNER_Y(r)  (WAVE_GRID_TOP + (r) * WAVE_CELL_H + 1)
#define WAVE_CENTER_Y(r) (WAVE_GRID_TOP + (r) * WAVE_CELL_H + WAVE_CELL_H / 2)

/* ------------------------------------------------------------------ */
/* Waveform palette indices (defined in initWaveBg in screens.c)        */
/* ------------------------------------------------------------------ */

#define PAL_BORDER    1   /* dim green   — borders and centre line      */
#define PAL_WAVE_NORM 2   /* bright green  — normal channel             */
#define PAL_WAVE_MUTE 3   /* very dim green — muted channel             */
#define PAL_WAVE_SOLO 4   /* bright yellow  — soloed channel            */

/* ------------------------------------------------------------------ */
/* Text-console overlay positions                                        */
/*                                                                       */
/* Each wave cell (64x46px) maps to 8 text cols x ~5.75 text rows.      */
/* Cell col c  -> text col base = c * 8                                  */
/* Cell row r label row  (top area):   { 2,  8, 13, 19 }               */
/* Cell row r status row (bottom area): { 5, 11, 17, 22 }               */
/* ------------------------------------------------------------------ */

static const u8 s_textLabelRow[4]  = {  2,  8, 13, 19 };
static const u8 s_textStatusRow[4] = {  5, 11, 17, 22 };

/* ------------------------------------------------------------------ */
/* Bitmap pointer                                                        */
/* ------------------------------------------------------------------ */

/* NDS VRAM must be accessed in 16-bit or 32-bit quantities.
   In 8bpp bitmap mode each u16 holds two adjacent pixels:
     bits  7:0  = left pixel  (even x)
     bits 15:8  = right pixel (odd  x)                                  */
static u16 *s_bmp = NULL;

static void bmpInit(void)
{
    if (s_bmp == NULL)
        s_bmp = (u16*)bgGetGfxPtr(sub_bg2);
}

/* ------------------------------------------------------------------ */
/* Low-level pixel helpers                                               */
/* ------------------------------------------------------------------ */

static void bmpClear(void)
{
    u32 *p = (u32*)s_bmp;
    for (u32 i = 0; i < 256 * 192 / 4; i++)
        p[i] = 0;
}

static inline void bmpPx(u8 x, u8 y, u8 c)
{
    u32  idx = (u32)y * 256 + x;
    u16 *p   = s_bmp + (idx >> 1);
    if (idx & 1)
        *p = (*p & 0x00FFu) | ((u16)c << 8);
    else
        *p = (*p & 0xFF00u) | c;
}

static void bmpHLine(u8 y, u8 x0, u8 x1, u8 c)
{
    u16 pair = ((u16)c << 8) | c;
    u32 base = (u32)y * 128;
    u8  xa = x0, xb = x1;

    if (xa & 1) {
        s_bmp[base + (xa >> 1)] = (s_bmp[base + (xa >> 1)] & 0x00FFu) | ((u16)c << 8);
        xa++;
    }
    if (xb < xa) return;
    if (!(xb & 1)) {
        s_bmp[base + (xb >> 1)] = (s_bmp[base + (xb >> 1)] & 0xFF00u) | c;
        if (xb == 0) return;
        xb--;
    }
    for (u8 xi = xa >> 1; xi <= (xb >> 1); xi++)
        s_bmp[base + xi] = pair;
}

static void bmpVLine(u8 x, u8 y0, u8 y1, u8 c)
{
    for (u8 y = y0; y <= y1; y++)
        bmpPx(x, y, c);
}

/* ------------------------------------------------------------------ */
/* Grid borders                                                          */
/* ------------------------------------------------------------------ */

static void drawWaveGrid(void)
{
    for (u8 r = 0; r <= 4; r++) {
        u8 y = WAVE_GRID_TOP + r * WAVE_CELL_H;
        if (y > 191) y = 191;
        bmpHLine(y, 0, 255, PAL_BORDER);
    }
    for (u8 c = 0; c <= 4; c++) {
        u8 x = c * WAVE_CELL_W;
        if (x > 255) x = 255;
        bmpVLine(x, WAVE_GRID_TOP, 191, PAL_BORDER);
    }
    for (u8 r = 0; r < 4; r++) {
        u8 cy = WAVE_CENTER_Y(r);
        for (u8 c = 0; c < 4; c++)
            bmpHLine(cy, WAVE_INNER_X(c), WAVE_INNER_X(c) + WAVE_INNER_W - 1, PAL_BORDER);
    }
}

/* ------------------------------------------------------------------ */
/* Change detection state                                               */
/* ------------------------------------------------------------------ */

static u8 s_lastInstrument[16];
static u8 s_lastNote[16];
static u8 s_lastVolume[16];

/* ------------------------------------------------------------------ */
/* Text overlay: channel label + mute/solo status                       */
/* ------------------------------------------------------------------ */

static void drawWaveCellText(u8 ch)
{
    u8 r        = ch / 4;
    u8 c        = ch % 4;
    u8 text_col = c * 8 + 1;  /* 1-char indent from left border */

    consoleSelect(&bottom);

    /* Channel label */
    iprintf("\x1b[%d;%dHCh.%-2d", s_textLabelRow[r], text_col, ch + 1);

    /* Mute / Solo status */
    const char *status = "     ";
    if (MODULE != NULL && ch < MODULE->NumberofChannels) {
        if (arm9_channelMute[ch])
            status = "Muted";
        else if (arm9_soloChannel == (s8)ch)
            status = " Solo";
    }
    iprintf("\x1b[%d;%dH%s", s_textStatusRow[r], text_col, status);
}

/* ------------------------------------------------------------------ */
/* Per-cell waveform drawing                                            */
/* ------------------------------------------------------------------ */

static void drawWaveCell(u8 ch)
{
    u8 r  = ch / 4;
    u8 c  = ch % 4;
    u8 x0 = WAVE_INNER_X(c);
    u8 y0 = WAVE_INNER_Y(r);
    u8 cy = WAVE_CENTER_Y(r);

    /* Choose waveform colour based on mute/solo state */
    u8 wave_color = PAL_WAVE_NORM;
    if (arm9_channelMute[ch])
        wave_color = PAL_WAVE_MUTE;
    else if (arm9_soloChannel == (s8)ch)
        wave_color = PAL_WAVE_SOLO;

    /* Clear inner area and restore centre line */
    for (u8 y = y0; y < y0 + WAVE_INNER_H; y++)
        bmpHLine(y, x0, x0 + WAVE_INNER_W - 1, 0);
    bmpHLine(cy, x0, x0 + WAVE_INNER_W - 1, PAL_BORDER);

    /* Update text overlay */
    drawWaveCellText(ch);

    if (MODULE == NULL) return;

    /* Navigate: channel -> instrument -> note -> sample */
    u8 inst_idx = MODULE->CurrentChannelLastInstrument[ch]; /* 1-based, 0=none */
    if (inst_idx == 0 || inst_idx > MODULE->NumberofInstruments) return;

    XM7_Instrument_Type *inst = MODULE->Instrument[inst_idx - 1];
    if (inst == NULL || inst->NumberofSamples == 0) return;

    u8 note       = MODULE->CurrentChannelLastNote[ch];   /* 1-based, 0=none */
    u8 sample_idx = (note > 0 && note <= 96)
                    ? inst->SampleforNote[note - 1] : 0;
    if (sample_idx >= inst->NumberofSamples) return;

    XM7_Sample_Type *samp = inst->Sample[sample_idx];
    if (samp == NULL || samp->Length == 0) return;

    /* Length is always in bytes; 16-bit samples need halving for sample count */
    bool is_16bit     = (samp->Flags & 0x10) != 0;
    u32  sample_count = is_16bit ? (samp->Length >> 1) : samp->Length;
    if (sample_count == 0) return;

    u32 step = sample_count / WAVE_INNER_W;
    if (step == 0) step = 1;

    s32 half     = WAVE_INNER_H / 2 - 1;           /* max pixel excursion from centre */
    u8  vol      = MODULE->CurrentSampleVolume[ch]; /* 0..0x40 — current envelope vol  */
    s32 eff_half = ((s32)half * vol) / 0x40;        /* amplitude scaled by envelope    */
    s32 max_amp  = is_16bit ? 32768 : 128;
    s32 prev_y_off = 0;

    for (u8 xi = 0; xi < WAVE_INNER_W; xi++) {
        u32 offset = (u32)xi * step;
        if (offset >= sample_count) offset = sample_count - 1;

        s32 val = is_16bit
                  ? (s32)samp->SampleData16->Data[offset]
                  : (s32)(s8)samp->SampleData->Data[offset];

        s32 y_off = (val * eff_half) / max_amp;
        if (y_off >  eff_half) y_off =  eff_half;
        if (y_off < -eff_half) y_off = -eff_half;

        /* y increases downward, so subtract offset from centre */
        u8 px  = x0 + xi;
        u8 py  = (u8)((s32)cy - y_off);
        u8 ppy = (u8)((s32)cy - prev_y_off);

        /* Connect current point to previous with a vertical segment */
        if (xi == 0 || py == ppy) {
            bmpPx(px, py, wave_color);
        } else if (py < ppy) {
            bmpVLine(px, py, ppy, wave_color);
        } else {
            bmpVLine(px, ppy, py, wave_color);
        }

        prev_y_off = y_off;
    }
}

/* ------------------------------------------------------------------ */
/* Touch handler                                                         */
/* ------------------------------------------------------------------ */

void handleWaveTouch(touchPosition *touchPos)
{
    float fx = (touchPos->rawx - X_MIN) * X_NORM;
    float fy = (touchPos->rawy - Y_MIN) * Y_NORM;
    u8 px    = (u8)(fx * SCREEN_WIDTH);
    u8 py    = (u8)(fy * SCREEN_HEIGHT);

    if (py < WAVE_GRID_TOP) return;

    u8 col = px / WAVE_CELL_W;
    u8 row = (py - WAVE_GRID_TOP) / WAVE_CELL_H;
    if (col > 3 || row > 3) return;

    u8 ch = row * 4 + col;
    if (MODULE == NULL || ch >= MODULE->NumberofChannels) return;

    if ((px % WAVE_CELL_W) < WAVE_CELL_W / 2) {
        /* Left half of cell: mute toggle */
        arm9_channelMute[ch] ^= 1;
        serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)ch << 8) | arm9_channelMute[ch]);
        drawWaveCell(ch);
    } else {
        /* Right half of cell: solo toggle */
        if (arm9_soloChannel == (s8)ch) {
            /* Un-solo: restore pre-solo mutes */
            arm9_soloChannel = -1;
            for (u8 i = 0; i < 16; i++) {
                arm9_channelMute[i] = arm9_preSoloMute[i];
                serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)i << 8) | arm9_channelMute[i]);
            }
        } else {
            /* Solo this channel: save current mutes if entering fresh */
            if (arm9_soloChannel < 0) {
                for (u8 i = 0; i < 16; i++)
                    arm9_preSoloMute[i] = arm9_channelMute[i];
            }
            arm9_soloChannel = (s8)ch;
            for (u8 i = 0; i < 16; i++) {
                arm9_channelMute[i] = (i != (u8)ch) ? 1 : 0;
                serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)i << 8) | arm9_channelMute[i]);
            }
        }
        for (u8 i = 0; i < 16; i++)
            drawWaveCell(i);
    }
}

/* ------------------------------------------------------------------ */
/* Public interface                                                      */
/* ------------------------------------------------------------------ */

void drawWaveScreen(void)
{
    bmpInit();
    bmpClear();
    drawWaveGrid();

    /* Invalidate change-detection state so every cell redraws on entry */
    memset(s_lastInstrument, 0xFF, sizeof(s_lastInstrument));
    memset(s_lastNote,       0xFF, sizeof(s_lastNote));
    memset(s_lastVolume,     0xFF, sizeof(s_lastVolume));

    /* Tab strip on BG0 first (consoleClear wipes previous text) */
    consoleSelect(&bottom);
    consoleClear();
    drawTabStrip(SCREEN_MODE_CH);

    /* Draw all cells — each call also writes its text overlay */
    for (u8 ch = 0; ch < 16; ch++)
        drawWaveCell(ch);

    /* Sync state to what we just drew so tick only redraws on real changes */
    if (MODULE != NULL) {
        for (u8 ch = 0; ch < 16; ch++) {
            s_lastInstrument[ch] = MODULE->CurrentChannelLastInstrument[ch];
            s_lastNote[ch]       = MODULE->CurrentChannelLastNote[ch];
            s_lastVolume[ch]     = MODULE->CurrentSampleVolume[ch];
        }
    }
}

void waveScreen_tick(void)
{
    if (s_bmp == NULL || MODULE == NULL) return;

    for (u8 ch = 0; ch < 16; ch++) {
        u8 inst = MODULE->CurrentChannelLastInstrument[ch];
        u8 note = MODULE->CurrentChannelLastNote[ch];
        u8 vol  = MODULE->CurrentSampleVolume[ch];

        if (inst != s_lastInstrument[ch] || note != s_lastNote[ch] || vol != s_lastVolume[ch]) {
            s_lastInstrument[ch] = inst;
            s_lastNote[ch]       = note;
            s_lastVolume[ch]     = vol;
            drawWaveCell(ch);
        }
    }
}
