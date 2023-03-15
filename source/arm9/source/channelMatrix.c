/*
 * channelMatrix.c
 *
 *  Created on: 20 gen 2023
 *      Author: merut
 */
#include <nds.h>
#include <stdio.h>
#include "screens.h"

#include "libXMX.h"
#include "libxm7.h"

void drawChannelStatus(u8 idx)
{
    consoleSelect(&bottom);
    iprintf("\x1b[%d;%dH%s", (idx / 4) * 6 + 3, (idx % 4) * 8 + 1, deckInfo.modManager->ChannelMute[idx] ? "Muted" : "     ");
}

void drawChannelMatrix()
{
    consoleSelect(&bottom);
    consoleClear();

    // Vertical separator
    for (u8 i = 0; i < 24; i++)
    {
        iprintf("\x1b[%d;0H|", i);
        iprintf("\x1b[%d;8H|", i);
        iprintf("\x1b[%d;16H|", i);
        iprintf("\x1b[%d;24H|", i);
    }

    // Horizontal separator
    for (u8 i = 0; i < 32; i++)
    {
        iprintf("\x1b[0;%dH-", i);
        iprintf("\x1b[6;%dH-", i);
        iprintf("\x1b[12;%dH-", i);
        iprintf("\x1b[18;%dH-", i);
    }

    // Print channel number
    for (u8 i = 0; i < 4; i++)
        for (u8 j = 0; j < 4; j++)
            iprintf("\x1b[%d;%dHCh.%d", 1 + i * 6, 2 + j * 8, i * 4 + j + 1);

    // Draw status (muted/unmuted)
    for (u8 i = 0; i < 16; i++)
        drawChannelStatus(i);
}

// Send mute
int8 handleChannelMute(touchPosition *touchPos)
{
    float x, y;
    x = (touchPos->rawx - X_MIN) * X_NORM;
    y = (touchPos->rawy - Y_MIN) * Y_NORM;

    u8 idx = 0;
    // Get idx from 0 to 15 based on x and y
    idx = (u8) (x * 4) + ((int) (y * 4) % 4) * 4;

    deckInfo.modManager->ChannelMute[idx] = !(deckInfo.modManager->ChannelMute[idx]);

    drawChannelStatus(idx);
    return idx;
}
