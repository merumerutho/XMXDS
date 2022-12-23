#include "nitroFSmenu.h"

#include <stdio.h>
#include <string.h>

#include "../../arm7/source/libxm7.h"

void XM7_FS_displayHeader()
{
    consoleClear();
    iprintf("-------------------------------\n");
    iprintf("File Select\n");
    iprintf("-------------------------------\n");
}

u8 XM7_FS_ls(DIR* folder, u16 pPosition)
{
    struct dirent* dirContent;
    u8 ff = 0;
    char pc = '>';

    seekdir(folder, ff % ENTRIES_PER_SCREEN);
    dirContent = readdir(folder);

    while (dirContent && ff <= ENTRIES_PER_SCREEN )
    {
        char p = (ff == (pPosition % ENTRIES_PER_SCREEN) ) ? pc : ' ';
        iprintf("\x1b[%d;%dH%c %s \n", (ff+3) % ENTRIES_PER_SCREEN, 0, p, dirContent->d_name);
        dirContent = readdir(folder);
        ff++;
    }
    rewinddir(folder);
    return ff;
}

struct dirent* getSelection(DIR* folder, u32 position)
{
    seekdir(folder, position);
    return readdir(folder);
}

void insertSlashIfNeeded(char* path)
{
    for(u8 i=1;i<254;i++)
    {
        if (path[i] == '\0' && path[i-1] != '/')
        {
            path[i] = '/';
            path[i+1] = '\0';
            break;
        }
    }
}

void navigateToFolder(DIR* folder, char* path, char* folderName)
{
    // Add slash if needed
    insertSlashIfNeeded(path);
    strcat(path, folderName);
    closedir(folder);
    folder = opendir(path);
}

void navigateBackwards(DIR* folder, char* path)
{
    closedir(folder);
    for(u8 i=255; i>=0; i--)
    {
        if(path[i] == '/' && path[i+1] != '\0')
        {
            path[i] = '\0';
            break;
        }
    }
    folder = opendir(path);
}

bool isXM(char* filename)
{
    for(u8 i=2;i<254;i++)
    {
        if (filename[i] == '\0')
        {
            return !(strcmp((char*) &(filename[i-2]), "xm"));
        }
    }
    return FALSE;  // this never happens
}

bool isMOD(char* filename)
{
    for(u8 i=2;i<254;i++)
    {
        if (filename[i] == '\0')
        {
            return !(strcmp((char*) &(filename[i-3]), "mod"));
        }
    }
    return FALSE;  // this never happens
}

void composeFileName(char* filepath, char* folder, char* filename)
{
    strcpy(filepath, folder);
    insertSlashIfNeeded(filepath);
    strcat(filepath, filename);
    iprintf("%s\n", filepath);
}

u32 XM7_FS_selectModule(char* folderPath)
{
    DIR* folder = opendir(folderPath);
    u32 pPosition = 0;
    u8 ff;
    bool bSelectingModule = TRUE;
    char filepath[255] = "";

    XM7_FS_displayHeader();
    ff = XM7_FS_ls(folder, pPosition);
    while(bSelectingModule)
    {
        scanKeys();
        if (keysDown() & KEY_UP)
        {
            pPosition = (pPosition==0) ? pPosition : pPosition - 1;
            ff = XM7_FS_ls(folder, pPosition);
        }

        if (keysDown() & KEY_DOWN)
        {
            pPosition = (pPosition == ff-1) ? pPosition : pPosition + 1;
            ff = XM7_FS_ls(folder, pPosition);
        }

        if (keysDown() & (KEY_L | KEY_R))
        {
            seekdir(folder, pPosition);
            struct dirent* selection = getSelection(folder, pPosition);

            if (selection->d_type == TYPE_FILE)
            {
                if(isXM(selection->d_name))
                {
                    composeFileName((char *)&filepath, folderPath, selection->d_name);
                    consoleClear();
                    return XM7_FS_loadModule(filepath, FS_TYPE_XM, ((keysDown() & KEY_L)==0));
                }
            }
        }

        if (keysDown() & (KEY_A))
        {
            seekdir(folder, pPosition);
            struct dirent* selection = getSelection(folder, pPosition);

            // If selection is a folder
            if (selection->d_type == TYPE_FOLDER)
            {
                if(!strcmp(selection->d_name, "."))
                    continue;
                if (!strcmp(selection->d_name, ".."))
                {
                    navigateBackwards(folder, folderPath);
                    pPosition = 0;
                    XM7_FS_displayHeader();
                    ff = XM7_FS_ls(folder, pPosition);
                }
                else
                {
                    navigateToFolder(folder, folderPath, selection->d_name);
                    pPosition = 0;
                    XM7_FS_displayHeader();
                    ff = XM7_FS_ls(folder, pPosition);
                }
            }
        }
    }
    return 0;
}

u32 XM7_FS_loadModule(char* filepath, u8 type, u8 slot)
{
    FILE* modFile = fopen(filepath, "rb");
    XM7_ModuleManager_Type * modManager = NULL;
    u32 size;

    fseek(modFile, 0L, SEEK_END);
    size = ftell(modFile);
    rewind(modFile);

    // Data destination
    void* modRawData = malloc(sizeof(u8) * (size));

    // Read data from file pointer
   if(fread(modRawData, sizeof(u8), size, modFile) != size)
       printf("\tCould not read module A correctly!\n");

   if (size > 0)
   {
       // allocate memory for the module
       modManager = malloc(sizeof(XM7_ModuleManager_Type));
       if (type == FS_TYPE_XM)
       {
           XM7_LoadXM(modManager, (XM7_XMModuleHeader_Type*) modRawData, slot);
       }

       // ensure data gets written to main RAM
       DC_FlushAll();
   }
   return (u32) modManager;
}

bool XM7_FS_init()
{
    // Load nitroFS file system
    return nitroFSInit(NULL);
}
