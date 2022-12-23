#include "libXMX.h"

XMX_ModuleInfo loadedModulesInfo[LIBXM7_ALLOWED_MODULES] =
{
    {
        .modData = NULL,
        .modManager = NULL,
        .moduleIndex = 0xFF
    },
    {
        .modData = NULL,
        .modManager = NULL,
        .moduleIndex = 0xFF
    }
};


void XMX_UnloadXM(u8 idx)
{
    XM7_UnloadXM(loadedModulesInfo[idx].modManager);
    free(loadedModulesInfo[idx].modData);
    loadedModulesInfo[idx].moduleIndex = 0xFF;
}
