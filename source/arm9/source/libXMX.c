#include "libXMX.h"

XMX_DeckInfo deckInfo =
        {
            .xmData = NULL,
            .modManager = NULL,
            .moduleIndex = 0,
        };

void XMX_UnloadXM()
{
    XM7_UnloadXM((XM7_ModuleManager_Type *)deckInfo.modManager);
    if (deckInfo.xmData != NULL)
        free((void*)deckInfo.xmData);
    if (deckInfo.modManager != NULL)
        free((void*)deckInfo.modManager);
}
