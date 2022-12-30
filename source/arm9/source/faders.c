#include "faders.h"
#include "libxmx.h"
#include "arm9_defines.h"

void evaluateVolumeFaders(u16 px, u16 py);
void evaluateCrossFader(u16 px, u16 py);

//

void evaluateFaders(touchPosition touchPos)
{
    u16 px = ((touchPos.rawx - 320) * SCREEN_WIDTH) / 3488;
    u16 py = SCREEN_HEIGHT - ((touchPos.rawy - 224) * SCREEN_HEIGHT) / 3680;
    evaluateVolumeFaders(px, py);
    evaluateCrossFader(px, py);
}

void drawVolumeFaders()
{
    // Draw position of volume bar on screen
    for (u8 i = 0; i < 13; i++)
    {
        // Deck A
        if (i == deckInfo[0].modManager->CurrentGlobalVolume / 10)
            iprintf("\x1b[%d;2H*", 20 - i);
        else
            iprintf("\x1b[%d;2H|", 20 - i);
        // Deck B
        if (i == deckInfo[1].modManager->CurrentGlobalVolume / 10)
            iprintf("\x1b[%d;28H*", 20 - i);
        else
            iprintf("\x1b[%d;28H|", 20 - i);
    }
    iprintf("\x1b[21;1H%3d", deckInfo[0].modManager->CurrentGlobalVolume);
    iprintf("\x1b[21;27H%3d", deckInfo[1].modManager->CurrentGlobalVolume);
}

void drawCrossFader()
{
    for (u8 i = 0; i < 20; i++)
    {
        if (i == (u8)(deckInfo[1].crossFaderVolume * 19))
            iprintf("\x1b[20;%dH*", 6 + i);
        else
            iprintf("\x1b[20;%dH-", 6 + i);
    }
}

void evaluateVolumeFaders(u16 px, u16 py)
{
    u8 targetModule = 0xFF;
    if (abs(px - 20) < 10) targetModule = 0;
    if (abs(px - (SCREEN_WIDTH - 20)) < 10) targetModule = 1;

    u16 value = (u16) (MIN(MAX(0, (py-30)*1.7), 170) * 127 / 170);
    if (targetModule < 2) deckInfo[targetModule].modManager->CurrentGlobalVolume = value;
}

void evaluateCrossFader(u16 px, u16 py)
{
    px = MIN(MAX(0,(px - 40)*1.1), (SCREEN_WIDTH-80));
    float value = (float) px / (SCREEN_WIDTH - 80);
    // Fix this
    if (abs(py - 25)<10 && abs(px - (SCREEN_WIDTH/2)) < (SCREEN_WIDTH - 80))
    {
        SetCrossFader(0, 1. - value);
        SetCrossFader(1, value);
    }
}
