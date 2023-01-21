#include "libXMX.h"

XMX_DeckInfo deckInfo[LIBXM7_ALLOWED_MODULES] = {
        {
            .modData = NULL,
            .modManager = NULL,
            .moduleIndex = 0xFF
        }
};

void XMX_UnloadXM(u8 idx)
{
    XM7_UnloadXM(deckInfo[idx].modManager);
    free(deckInfo[idx].modData);
    deckInfo[idx].moduleIndex = 0xFF;
}
