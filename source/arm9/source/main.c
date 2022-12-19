#include <nds.h>
#include <stdio.h>

#include <filesystem.h>
#include <dirent.h>

#include "../../arm7/source/libxm7.h"

// FIFO 07 for libxm7
#define FIFO_XM7    (FIFO_USER_07)
#define MOD_TO_PLAY "data/mods/demo_C.xm"

//---------------------------------------------------------------------------------
void XM7_arm9_Value32Handler (u32 command, void* userdata)
{
    if (command)
        // received a pointer to a module that should start now
        iprintf("%ld\n", command);
    return;
}

int getFolderName(char* result, char* filepath)
{
    char c = filepath[0];
    u8 i, lastKnownSlashPosition = 0;
    for(i=0 ; c!='\0' ; c=filepath[i++])
    {
        if (c == '/')
            lastKnownSlashPosition = i;
    }
    memcpy(result, filepath, lastKnownSlashPosition+1);
    result[i] = '\0';

    return(0);
}

void displayIntro()
{
    iprintf("     .oO:Oo. XMXDS .oO:Oo.\n\n");
    iprintf(" 2-decks module player for NDS!\n");
    iprintf(" credits: @merumerutho\n");
    iprintf(" based on libxm7 (@sverx)\n\n");
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // touchPosition touch;
    consoleDemoInit();  // setup the sub screen for printing

    u16 playing = 0;

    XM7_ModuleManager_Type* module_A = NULL;

    FILE* modA_file;

    long fsz = 0;
    char path[255];
    getFolderName(path, argv[0]);

    //iprintf("%s\n", path);

	if(! nitroFSInit(NULL))  // load nitro FileSystem
    {
        iprintf("Could not load nitroFS!\n");
        // Wait for key then exit
        while(1)
        {
            scanKeys();
            if (keysDown())
                return(1);
        }
    }

	// :)
	displayIntro();

    iprintf("\tLoading %s...\n", MOD_TO_PLAY);
    // File reading check
    modA_file = fopen(MOD_TO_PLAY, "rb");

    fseek(modA_file, 0L, SEEK_END);
    // Get file size
    fsz = ftell(modA_file);
    rewind(modA_file);

    // Prepare destination
    u8* modA_data = malloc(sizeof(u8) * (fsz));

    // Read XM raw data
    if(fread(modA_data, sizeof(u8), fsz, modA_file) != fsz)
        printf("\tCould not read the module correctly!\n");

    if (fsz > 0)
    {
        iprintf("\tFound %s!\n", MOD_TO_PLAY);
        iprintf("\tLoaded %ld bytes\n", fsz);
        // allocate memory for the module
        module_A = malloc(sizeof(XM7_ModuleManager_Type));
        u16 res = XM7_LoadXM(module_A, (XM7_XMModuleHeader_Type*) modA_data, 0);

        // ensure data gets written to main RAM
        DC_FlushAll();

        if (res!=0)
            iprintf("\tLoading went wrong for error 0x%04x\n",res);
    }

    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);

    // Install the FIFO handler for libXM7 "fifo channel"
    fifoSetValue32Handler(FIFO_XM7, XM7_arm9_Value32Handler, NULL);

    iprintf("\n\tPress A to play, B to stop.\n");
    while(1) 
    {
        // read keys
        scanKeys();
        if (keysDown() & KEY_A)
        {
            // sending pointer to the libxm7 engine on ARM7
            if (!fifoSendValue32(FIFO_XM7, (u32)module_A))
                iprintf("Could not send data correctly...\n");

            playing= 1-playing;
            if (playing)
                iprintf(" Playing... ");
            else
                iprintf(" Stop.\n");
        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
    
    /*
	while(1) {

		touchRead(&touch);
		iprintf("\x1b[10;0HTouch x = %04i, %04i\n", touch.rawx, touch.px);
		iprintf("Touch y = %04i, %04i\n", touch.rawy, touch.py);

		swiWaitForVBlank();
		scanKeys();
		if (keysDown()&KEY_START) break;
	}
    */
	return 0;
}
