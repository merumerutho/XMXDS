/*
 * channelMatrix.c
 */
#include <nds.h>
#include <stdio.h>
#include "screens.h"
#include "arm9_fifo.h"

void drawChannelStatus(u8 idx)
{
    consoleSelect(&bottom);
    iprintf("\x1b[%d;%dH%s", (idx / 4) * 6 + 3, (idx % 4) * 8 + 1,
            arm9_channelMute[idx] ? "Muted" : "     ");
}

void drawChannelMatrix()
{
    consoleSelect(&bottom);
    consoleClear();

    // Vertical separators
    for (u8 i = 0; i < 24; i++)
    {
        iprintf("\x1b[%d;0H|", i);
        iprintf("\x1b[%d;8H|", i);
        iprintf("\x1b[%d;16H|", i);
        iprintf("\x1b[%d;24H|", i);
    }

    // Horizontal separators
    for (u8 i = 0; i < 32; i++)
    {
        iprintf("\x1b[0;%dH-", i);
        iprintf("\x1b[6;%dH-", i);
        iprintf("\x1b[12;%dH-", i);
        iprintf("\x1b[18;%dH-", i);
    }

    // Channel numbers
    for (u8 i = 0; i < 4; i++)
        for (u8 j = 0; j < 4; j++)
            iprintf("\x1b[%d;%dHCh.%d", 1 + i * 6, 2 + j * 8, i * 4 + j + 1);

    // Mute status
    for (u8 i = 0; i < 16; i++)
        drawChannelStatus(i);
}

int8 handleChannelMute(touchPosition *touchPos)
{
    float x = (touchPos->rawx - X_MIN) * X_NORM;
    float y = (touchPos->rawy - Y_MIN) * Y_NORM;

    u8 idx = (u8)(x * 4) + ((int)(y * 4) % 4) * 4;

    // Toggle shadow state and send to ARM7
    arm9_channelMute[idx] ^= 1;
    serviceCmd(CMD_SET_CHANNEL_MUTE, ((s32)idx << 8) | arm9_channelMute[idx]);

    drawChannelStatus(idx);
    return idx;
}
