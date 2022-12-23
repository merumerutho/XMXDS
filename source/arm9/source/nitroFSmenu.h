#ifndef ARM9_SOURCE_NITROFSMENU_H_
#define ARM9_SOURCE_NITROFSMENU_H_

#include <nds.h>
#include <dirent.h>
#include <filesystem.h>

#define ENTRIES_PER_SCREEN  20

#define TYPE_FOLDER         4
#define TYPE_FILE           8

void XM7_FS_selectModule(char* folder);

#endif /* ARM9_SOURCE_NITROFSMENU_H_ */
