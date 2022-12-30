#include "faders.h"
#include "libxmx.h"
#include "arm9_defines.h"

void evaluateVolumeFadersAndCrossFader(touchPosition touchPos)
{
    u8 targetModule = 0xFF;
    u16 px = ((touchPos.rawx - 320) * SCREEN_WIDTH) / 3488;
    u16 py = SCREEN_HEIGHT - ((touchPos.rawy - 224) * SCREEN_HEIGHT) / 3680;

    if (abs(px - 20) < 10) targetModule = 0;
    if (abs(px - (SCREEN_WIDTH - 20)) < 10) targetModule = 1;

    u16 value = (u16) (MIN(MAX(0, (py-30)*1.7), 170) * 127 / 170);
    if(targetModule < 2)
        loadedModulesInfo[targetModule].modManager->CurrentGlobalVolume = value;
}

void drawVolumeFader()
{
    // Draw position of volume bar on screen
    for (u8 i=0; i < 13; i++)
    {
        if (i==loadedModulesInfo[0].modManager->CurrentGlobalVolume/10)
            iprintf("\x1b[%d;2H*", 20 - i);
        else
            iprintf("\x1b[%d;2H|", 20 - i);
        if (i==loadedModulesInfo[1].modManager->CurrentGlobalVolume/10)
            iprintf("\x1b[%d;28H*", 20 - i);
        else
            iprintf("\x1b[%d;28H|", 20 - i);
    }
    iprintf("\x1b[21;1H%3d", loadedModulesInfo[0].modManager->CurrentGlobalVolume);
    iprintf("\x1b[21;27H%3d", loadedModulesInfo[1].modManager->CurrentGlobalVolume);
}
