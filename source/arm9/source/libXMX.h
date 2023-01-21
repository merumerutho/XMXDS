#ifndef ARM9_SOURCE_LIBXMX_H_
#define ARM9_SOURCE_LIBXMX_H_

#include <nds.h>
#include "../../arm7/source/libxm7.h"

typedef struct
{
    XM7_XMModuleHeader_Type *modData;
    XM7_ModuleManager_Type *modManager;
    u8 moduleIndex;

} __attribute__ ((packed)) XMX_DeckInfo;

// ...

extern XMX_DeckInfo deckInfo[LIBXM7_ALLOWED_MODULES];

void XMX_UnloadXM(u8 idx);

#endif /* ARM9_SOURCE_LIBXMX_H_ */
