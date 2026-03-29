#include "libXMX.h"

XMX_DeckInfo deckInfo =
        {
            .xmData = NULL,
            .modManager = NULL,
            .moduleIndex = 0,
        };

void XMX_UnloadXM()
{
    if (deckInfo.modManager != NULL)
    {
        XM7_UnloadXM((XM7_ModuleManager_Type *)deckInfo.modManager);
        free((void*)deckInfo.modManager);
        deckInfo.modManager = NULL;
    }
    if (deckInfo.xmData != NULL)
    {
        free((void*)deckInfo.xmData);
        deckInfo.xmData = NULL;
    }
}
