#include <stdio.h>
#include <string.h>

#include "filesystem.h"
#include "play.h"
#include "libXMX.h"
#include "screens.h"

#include "arm9_fifo.h"

void XMX_FileSystem_displayHeader()
{
    consoleSelect(&bottom);
    consoleClear();
    iprintf("-------------------------------\n");
    iprintf(" XMXDS - File browser\n");
    iprintf("-------------------------------\n");
}

// Function to count up to UINT16_MAX files in a folder
u16 getFileCount(DIR *folder)
{
    u16 counter = 0;
    rewinddir(folder);
    for (counter = 0; counter < 0xFFFF; counter++)
        if (readdir(folder) == NULL) break;
    rewinddir(folder);
    return counter;
}

void my_snprintf(char *dest, char *src, u8 len, bool fillWSpace)
{
    u8 c;
    // Regular char copy up to declared length
    for (c = 0; c < len; c++)
    {
        if (src[c] == '\0') break;
        dest[c] = src[c];
    }
    // If string is longer than length...
    if (c == len)
    {
        // Apply ugliest fuckery ever conceived to show that the string was cut
        dest[len - 2] = '~';
        dest[len - 1] = '\0';
    }
    // Fill with spaces if required
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
    // In any other case apply termination character at latest index
    else
        dest[c] = '\0';
}

void listFolderOnPosition(DIR *folder, u16 pPosition, u16 fileCount)
{
    struct dirent *dirContent;
    u8 ff = 0;
    char pc = '>';
    char dir[4] = "dir";

    consoleSelect(&bottom);

    // pPosition represents the pointed file position,
    // and since we show ENTRIES_PER_SCREEN file at every moment,
    // the file seek is based on integer multiples of ENTRIES_PER_SCREEN
    seekdir(folder, (u32) (pPosition / ENTRIES_PER_SCREEN) * ENTRIES_PER_SCREEN);
    dirContent = readdir(folder);
    char filename[20] = "";

    while (dirContent && ff < ENTRIES_PER_SCREEN)
    {
        char p = (ff == (pPosition % ENTRIES_PER_SCREEN)) ? pc : ' ';
        char *d = (dirContent->d_type == TYPE_FOLDER) ? dir : "   ";

        // File name is cut to fit in 20 chars
        my_snprintf(filename, dirContent->d_name, 20, TRUE);

        consoleSelect(&bottom);
        // Print filename
        iprintf("\x1b[%d;%dH%c %s %s \n", ff + 4, 0, p, d, filename);
        dirContent = readdir(folder);
        ff++;
    }
    // Fill rest with empty spaces to clear the remaining part of the screen
    while (ff < ENTRIES_PER_SCREEN)
    {
        consoleSelect(&bottom);
        iprintf("\x1b[%d;%dH  %s \n", ff + 4, 0, "                               ");
        ff++;
    }
    // Rewind folder for next read
    rewinddir(folder);

    // Print current file position
    consoleSelect(&bottom);
    iprintf("\x1b[21;0H-------------------------------");
    // Spaces in the string are used to clean if screen gets dirty
    iprintf("\x1b[22;0H%d/%d          ", pPosition + 1, fileCount);
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

void composeFileName(char *filepath, char *folder, char *filename)
{
    strcpy(filepath, folder);
    insertSlashAtEnd(filepath);
    strcat(filepath, filename);
    consoleSelect(&bottom);
    iprintf("%s\n", filepath);
}

u8 XMX_FileSystem_selectModule(char *folderPath)
{
    DIR *folder = opendir(folderPath);
    u16 fileCount;
    u32 pPosition = 0;
    bool bSelectingModule = TRUE;
    char filepath[255] = "";

    XMX_FileSystem_displayHeader();

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
            return 1;  // User exit
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
                    XMX_FileSystem_displayHeader();
                    fileCount = getFileCount(folder);
                    listFolderOnPosition(folder, pPosition, fileCount);
                }
                else
                {
                    navigateToFolder(folder, folderPath, selection->d_name);
                    pPosition = 0;
                    XMX_FileSystem_displayHeader();
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
                // If file selected is a module
                if (isXM(selection->d_name))
                {
                    composeFileName((char*) &filepath, folderPath, selection->d_name);
                    consoleClear();

                    // If deck already loaded
                    if (deckInfo.modManager != NULL)
                    {
                        // Stop if playing
                        if (deckInfo.modManager->State == XM7_STATE_PLAYING)
                            play_stop();
                        // Unload module TODO: (this must be moved to a dedicated function)
                        {
                            XMX_UnloadXM();
                            // Reset Hot cue position to 0
                            arm9_globalHotCuePosition = 0;
                        }
                    }
                    deckInfo.modManager = malloc(sizeof(XM7_ModuleManager_Type));

                    // LOADING MODULE
                    deckInfo.xmData = (XM7_XMModuleHeader_Type*)
                                            XMX_FileSystem_loadModule(deckInfo.modManager, filepath);
                    // MODULE NAME
                    if (deckInfo.xmData != NULL)
                    {
                        my_snprintf(deckInfo.modManager->ModuleName, selection->d_name, 16, FALSE);
                        closedir(folder);
                        return 0; // Loaded successfully
                    }
                    else
                    {
                        consoleSelect(&bottom);
                        iprintf("Could not load module!");
                        closedir(folder);
                        return 2; // Unable to load
                    }
                }
            }
        }
        swiWaitForVBlank();
    }
    closedir(folder);
    return 3; // should never happen
}

void* XMX_FileSystem_loadModule(XM7_ModuleManager_Type *pMod, char *filepath)
{
    FILE *moduleFile = fopen(filepath, "rb");
    u16 ret;

    // Get size
    fseek(moduleFile, 0L, SEEK_END);
    u32 size = ftell(moduleFile);
    rewind(moduleFile);

    // Data destination
    XM7_XMModuleHeader_Type *modHeader = malloc(sizeof(u8) * (size));

    // Read data from file pointer and write it to modHeader
    if (fread((void *)modHeader, sizeof(u8), size, moduleFile) != size)
    {
        printf("\tCould not read file correctly!\n");
        return NULL;
    }
    fclose(moduleFile);

    // Load module
    if (size > 0)
    {
        // TODO: this must be moved to a function somewhere else
        {
            ret = XM7_LoadXM(pMod, modHeader);
            if (ret > 0) return NULL;

            iprintf("Done.");
            while (1)
            {
                scanKeys();
                if (keysDown()) break;
            }

            // Update bpm, tempo, hotcuepos (no longer done inside XM7_LoadXM)
            arm9_globalBpm = pMod->DefaultBPM;
            arm9_globalTempo = pMod->DefaultTempo;
            arm9_globalHotCuePosition = pMod->CurrentSongPosition;

            // Ensure data gets written to main RAM (leave no data in cache)
            DC_FlushAll();
        }
    }

    return modHeader;
}

bool XMX_FileSystem_init()
{
    // Load nitroFS file system
    return nitroFSInit(NULL);
}
