#ifndef ARM9_SOURCE_LIBXMX_H_
#define ARM9_SOURCE_LIBXMX_H_

#include <nds.h>
#include "libxm7.h"

typedef struct
{
    u8 moduleIndex;

    XM7_XMModuleHeader_Type *xmData;
    XM7_ModuleManager_Type *modManager;

} __attribute__ ((packed)) XMX_DeckInfo;

// ...

extern XMX_DeckInfo deckInfo;

void XMX_UnloadXM();

#endif /* ARM9_SOURCE_LIBXMX_H_ */
