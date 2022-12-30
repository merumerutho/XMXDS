#include "libXMX.h"

XMX_ModuleInfo deckInfo[LIBXM7_ALLOWED_MODULES] = {
        {
            .modData = NULL,
            .modManager = NULL,
            .moduleIndex = 0xFF,

            .crossFaderVolume = 0.5
        },
        {
            .modData = NULL,
            .modManager = NULL,
            .moduleIndex = 0xFF,

            .crossFaderVolume = 0.5
        }
};

void XMX_UnloadXM(u8 idx)
{
    XM7_UnloadXM(deckInfo[idx].modManager);
    free(deckInfo[idx].modData);
    deckInfo[idx].moduleIndex = 0xFF;
}

void SetCrossFader(u8 idx, float value)
{
    deckInfo[idx].crossFaderVolume = value;
    deckInfo[idx].modManager->CrossFaderVolume = value;
}
