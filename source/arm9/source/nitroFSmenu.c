#include "nitroFSmenu.h"

#include <stdio.h>
#include <string.h>

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

void navigateToFolder(DIR* folder, char* path, char* folderName)
{
    // Add slash if needed
    for(u8 i=0;i<255;i++)
    {
        if (path[i] == '\0' && path[i-1] != '/' && i!=0 && i!=254)
        {
            path[i] = '/';
            path[i+1] = '\0';
            break;
        }
    }
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

void XM7_FS_selectModule(char* folderPath)
{
    DIR* folder = opendir(folderPath);
    u32 pPosition = 0;
    u8 ff;
    bool bSelectingModule = TRUE;

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

        if (keysDown() & KEY_A)
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
            // If selection is a file
            else if (selection->d_type == TYPE_FILE)
            {
                // Evaluate file?
                bSelectingModule = FALSE;
            }
        }
    }
}
