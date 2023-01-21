#include "nitroFSmenu.h"

#include <stdio.h>
#include <string.h>

#include "play.h"
#include "libXMX.h"
#include "screens.h"

//#include "../../arm7/source/libxm7.h"

void XM7_FS_displayHeader()
{
    consoleSelect(&bottom);
    consoleClear();
    iprintf("-------------------------------\n");
    iprintf(" XMXDS - File browser\n");
    iprintf("-------------------------------\n");
}

u16 getFileCount(DIR *folder)
{
    u16 counter = 0;
    rewinddir(folder);
    for (counter = 0; counter < 0xFFFF; counter++)
        if (readdir(folder) == NULL) break;
    rewinddir(folder);
    return counter;
}

void strcpy_cut(char *dest, char *src, u8 len, bool fillWSpace)
{
    u8 c;
    for (c = 0; c < len; c++)
    {
        if (src[c] == '\0') break;
        dest[c] = src[c];
    }
    // If file was too long
    if (c == len)
    {
        dest[len - 2] = '~'; // Ugliest fuckery ever made to show file has a long name
        dest[len - 1] = '\0';
    }
    else if (fillWSpace)
    {
        for (; c < len; c++)
        {
            if (c == len - 1)
                dest[c] = '\0';
            else
                dest[c] = ' ';
        }
    }
    else
        dest[c] = '\0';
}

void listFolderOnPosition(DIR *folder, u16 pPosition, u16 fileCount)
{
    struct dirent *dirContent;
    u8 ff = 0;
    char pc = '>';
    char dir[4] = "dir";

    if (pPosition > 255) iprintf("ERROR\nNot supported!\n");

    seekdir(folder, (u32) (pPosition & ENTRIES_PER_SCREEN));
    dirContent = readdir(folder);
    char filename[20] = "";

    while (dirContent && ff < ENTRIES_PER_SCREEN)
    {
        char p = (ff == (pPosition % ENTRIES_PER_SCREEN)) ? pc : ' ';
        char *d = (dirContent->d_type == TYPE_FOLDER) ? dir : "   ";
        strcpy_cut(filename, dirContent->d_name, 20, TRUE);
        iprintf("\x1b[%d;%dH%c %s %s \n", ff + 4, 0, p, d, filename);
        dirContent = readdir(folder);
        ff++;
    }
    while (ff < ENTRIES_PER_SCREEN)
    {
        iprintf("\x1b[%d;%dH  %s \n", ff + 4, 0, "                               ");
        ff++;
    }

    rewinddir(folder);
    iprintf("\x1b[21;0H-------------------------------");
    iprintf("\x1b[22;0H%03d/%03d", pPosition + 1, fileCount);
    iprintf("\x1b[23;0H-------------------------------");
}

struct dirent* getSelection(DIR *folder, u32 position)
{
    seekdir(folder, position);
    return readdir(folder);
}

void insertSlashAtEnd(char *path)
{
    for (u8 i = 1; i < 254; i++)
    {
        if (path[i] == '\0' && path[i - 1] != '/')
        {
            path[i] = '/';
            path[i + 1] = '\0';
            break;
        }
    }
}

void navigateToFolder(DIR *folder, char *path, char *folderName)
{
    // Add slash if needed
    insertSlashAtEnd(path);
    strcat(path, folderName);
    closedir(folder);
    folder = opendir(path);
}

void navigateBackwards(DIR *folder, char *path)
{
    closedir(folder);
    u8 i = 0, counter = 0;

    // Count how many slashes present
    for (u8 i = 0; i < 255; i++)
    {
        if (path[i] == '/') counter++;
        if (path[i] == '\0') break;
    }

    // Remove stuff only if there is at least one slash
    if (counter > 0)
    {
        for (u8 k = i; k >= 0; k--)
        {
            if (path[k] == '/' && path[k + 1] != '\0')
            {
                path[k] = '\0';
                break;
            }
        }
    }
    // Otherwise, go to root folder
    else
        strcpy(path, ".");

    folder = opendir(path);
}

bool isXM(char *filename)
{
    for (u8 i = 2; i < 254; i++)
        if (filename[i] == '\0') return !(strcmp((char*) &(filename[i - 2]), "xm"));
    return FALSE;  // this never happens
}

bool isMOD(char *filename)
{
    for (u8 i = 2; i < 254; i++)
        if (filename[i] == '\0') return !(strcmp((char*) &(filename[i - 3]), "mod"));
    return FALSE;  // this never happens
}

void composeFileName(char *filepath, char *folder, char *filename)
{
    strcpy(filepath, folder);
    insertSlashAtEnd(filepath);
    strcat(filepath, filename);
    iprintf("%s\n", filepath);
}

/*
 *
 */
void XM7_FS_selectModule(char *folderPath)
{
    DIR *folder = opendir(folderPath);
    u16 fileCount;
    u32 pPosition = 0;
    bool bSelectingModule = TRUE;
    char filepath[255] = "";

    XM7_FS_displayHeader();

    fileCount = getFileCount(folder);
    listFolderOnPosition(folder, pPosition, fileCount);
    keysSetRepeat(20, 4);
    while (bSelectingModule)
    {
        scanKeys();
        u32 kdr = keysDownRepeat();

        if (kdr & KEY_UP)
        {
            pPosition = (pPosition == 0) ? pPosition : pPosition - 1;
            listFolderOnPosition(folder, pPosition, fileCount);
        }

        if (kdr & KEY_DOWN)
        {
            pPosition = (pPosition == fileCount - 1) ? pPosition : pPosition + 1;
            listFolderOnPosition(folder, pPosition, fileCount);
        }

        if (keysDown() & KEY_RIGHT)
        {
            pPosition = ((pPosition + ENTRIES_PER_SCREEN) > (fileCount - 1)) ? fileCount - 1 : pPosition + ENTRIES_PER_SCREEN;
            listFolderOnPosition(folder, pPosition, fileCount);
        }

        if (keysDown() & KEY_LEFT)
        {
            pPosition = ((int32) (pPosition - ENTRIES_PER_SCREEN) < 0) ? 0 : pPosition - ENTRIES_PER_SCREEN;
            listFolderOnPosition(folder, pPosition, fileCount);
        }

        if (keysDown() & (KEY_B))
        {
            return;
        }

        if (keysDown() & (KEY_A | KEY_L | KEY_R))
        {
            seekdir(folder, pPosition);
            struct dirent *selection = getSelection(folder, pPosition);

            // If selection is a folder
            if (selection->d_type == TYPE_FOLDER)
            {
                if (!strcmp(selection->d_name, ".")) continue;
                if (!strcmp(selection->d_name, ".."))
                {
                    navigateBackwards(folder, folderPath);
                    pPosition = 0;
                    XM7_FS_displayHeader();
                    fileCount = getFileCount(folder);
                    listFolderOnPosition(folder, pPosition, fileCount);
                }
                else
                {
                    navigateToFolder(folder, folderPath, selection->d_name);
                    pPosition = 0;
                    XM7_FS_displayHeader();
                    fileCount = getFileCount(folder);
                    listFolderOnPosition(folder, pPosition, fileCount);
                }
            }
        }

        if (keysDown() & KEY_A)
        {
            struct dirent *selection = getSelection(folder, pPosition);

            if (selection->d_type == TYPE_FILE)
            {
                if (isXM(selection->d_name))
                {
                    composeFileName((char*) &filepath, folderPath, selection->d_name);
                    consoleClear();
                    // Stop module if playing, unload module if present
                    if (deckInfo[0].modManager != NULL)
                    {
                        if (deckInfo[0].modManager->State == XM7_STATE_PLAYING) play_stop(&deckInfo[0]);
                        XMX_UnloadXM(0);
                    }
                    deckInfo[0].modManager = malloc(sizeof(XM7_ModuleManager_Type));
                    deckInfo[0].moduleIndex = 0;
                    // Load module
                    deckInfo[0].modData = XM7_FS_loadModule(deckInfo[0].modManager, filepath,
                                                            FS_TYPE_XM,
                                                            0);
                    if (deckInfo[0].modData != NULL)
                        strcpy_cut(deckInfo[0].modManager->ModuleName, selection->d_name, 16, FALSE);
                    else
                        iprintf("Could not load module!");
                    return;
                }
            }
        }
        swiWaitForVBlank();
    }
    return;
}

XM7_XMModuleHeader_Type* XM7_FS_loadModule(XM7_ModuleManager_Type *pMod, char *filepath, u8 type, u8 slot)
{
    FILE *modFile = fopen(filepath, "rb");
    u32 size;
    u16 ret;

    fseek(modFile, 0L, SEEK_END);
    size = ftell(modFile);
    rewind(modFile);

    // Data destination
    void *modHeader = malloc(sizeof(u8) * (size));

    // Read data from file pointer
    if (fread(modHeader, sizeof(u8), size, modFile) != size) printf("\tCould not read module correctly!\n");

    if (size > 0)
    {
        if (type == FS_TYPE_XM)
        {
            ret = XM7_LoadXM(pMod, (XM7_XMModuleHeader_Type*) modHeader, slot);
            if (ret>0)
                return NULL;
        }
        // Ensure data gets written to main RAM (leave no data in cache)
        DC_FlushAll();
    }
    return modHeader;
}

bool XM7_FS_init()
{
    // Load nitroFS file system
    return nitroFSInit(NULL);
}
