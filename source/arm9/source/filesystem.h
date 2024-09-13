#ifndef ARM9_SOURCE_FILESYSTEM_H_
#define ARM9_SOURCE_FILESYSTEM_H_

#include <nds.h>
#include <dirent.h>
#include <filesystem.h>

#include "libXMX.h"

#include "libxm7.h"

#define ENTRIES_PER_SCREEN  16

#define FS_TYPE_XM          0
#define FS_TYPE_MOD         1

#define TYPE_FOLDER         DT_DIR

bool XMX_FileSystem_init();
u8 XMX_FileSystem_selectModule(char *folderPath);
void* XMX_FileSystem_loadModule(XM7_ModuleManager_Type *pMod, char *filepath);

#endif /* ARM9_SOURCE_FILESYSTEM_H_ */
