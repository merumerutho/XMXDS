#ifndef ARM9_SOURCE_NITROFSMENU_H_
#define ARM9_SOURCE_NITROFSMENU_H_

#include <nds.h>
#include <dirent.h>
#include <filesystem.h>

#include "libXMX.h"

#include "../../arm7/source/libxm7.h"

#define ENTRIES_PER_SCREEN  16

#define FS_TYPE_XM          0
#define FS_TYPE_MOD         1

#define TYPE_FOLDER         4
#define TYPE_FILE           8

bool XM7_FS_init();
u8 XM7_FS_selectModule(char *folderPath);
void doNothing();
XM7_XMModuleHeader_Type* XM7_FS_loadModule(XM7_ModuleManager_Type *pMod, char *filepath, u8 type, u8 slot);

#endif /* ARM9_SOURCE_NITROFSMENU_H_ */
