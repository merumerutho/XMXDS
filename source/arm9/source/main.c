#include <nds.h>
#include <stdio.h>
#include <filesystem.h>
#include <dirent.h>

// ARMv7 INCLUDES
#include "../../arm7/source/libxm7.h"
#include "../../arm7/source/tempo.h"

// FIFO 07 for libxm7
#define FIFO_XM7    (FIFO_USER_07)

#define MOD_TO_PLAY_A "data/mods/demo_D.xm"
#define MOD_TO_PLAY_B "data/mods/demo_E.xm"

u8 arm9_globalBpm = 125;
u8 arm9_globalTempo = 6;

bool playingA;
bool playingB;

//---------------------------------------------------------------------------------
void XM7_arm9_Value32Handler (u32 command, void* userdata)
{
    if (command)
        // received a pointer to a module that should start now
        iprintf("%ld\n", command);
    return;
}

void displayIntro()
{
    iprintf("     .oO:Oo. XMXDS .oO:Oo.\n\n");
    iprintf(" 2-decks module player for NDS!\n");
    iprintf(" credits: @merumerutho\n");
    iprintf(" based on libxm7 (@sverx)\n\n");
}

void play_stop(XM7_ModuleManager_Type* module)
{
    // sending pointer to the libxm7 engine on ARM7
    if (!fifoSendValue32(FIFO_XM7, (u32) module))
        iprintf("Could not send data correctly...\n");

    playingA = (module->moduleIndex == 0) ? !playingA : playingA;

    playingB = (module->moduleIndex == 1) ? !playingB : playingB;
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    // touchPosition touch;
    consoleDemoInit();  // setup the sub screen for printing

    XM7_ModuleManager_Type* module_A = NULL;
    XM7_ModuleManager_Type* module_B = NULL;

    FILE* modA_file;
    FILE* modB_file;

    long fszA = 0, fszB = 0;

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

    // File reading check
    modA_file = fopen(MOD_TO_PLAY_A, "rb");

    fseek(modA_file, 0L, SEEK_END);
    // Get file size
    fszA = ftell(modA_file);
    rewind(modA_file);
    iprintf("%ld bytes", fszA);

    modB_file = fopen(MOD_TO_PLAY_B, "rb");
    fseek(modB_file, 0L, SEEK_END);
    // Get file size
    fszB = ftell(modB_file);
    rewind(modB_file);
    iprintf("%ld bytes", fszB);

    // Prepare destination
    u8* modA_data = malloc(sizeof(u8) * (fszA));
    u8* modB_data = malloc(sizeof(u8) * (fszB));

    // Read XM raw data
    if(fread(modA_data, sizeof(u8), fszA, modA_file) != fszA)
        printf("\tCould not read module A correctly!\n");

    if(fread(modB_data, sizeof(u8), fszB, modB_file) != fszB)
            printf("\tCould not read module B correctly!\n");

    if (fszA > 0 && fszB > 0)
    {
        iprintf("\tFound %s and %s.\n", MOD_TO_PLAY_A, MOD_TO_PLAY_B);
        iprintf("\tLoaded %ld bytes\n", fszA);
        iprintf("\tLoaded %ld bytes\n", fszB);
        // allocate memory for the module
        module_A = malloc(sizeof(XM7_ModuleManager_Type));
        module_B = malloc(sizeof(XM7_ModuleManager_Type));
        u16 res = XM7_LoadXM(module_A, (XM7_XMModuleHeader_Type*) modA_data, 0);
        // ensure data gets written to main RAM
        DC_FlushAll();

        if (res!=0)
            iprintf("\tLoading went wrong for module A, error 0x%04x\n",res);

        res = XM7_LoadXM(module_B, (XM7_XMModuleHeader_Type*) modB_data, 1);
        // ensure data gets written to main RAM
        DC_FlushAll();

        if (res!=0)
            iprintf("\tLoading went wrong for module B, error 0x%04x\n",res);

    }

    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);

    // Install the FIFO handler for libXM7 "fifo channel"
    fifoSetValue32Handler(FIFO_XM7, XM7_arm9_Value32Handler, NULL);

    u32 word;

    word = (FIFO_TARGET_BPM << 8) | arm9_globalBpm;
    fifoSendValue32(FIFO_TEMPO, word);

    word = (FIFO_TARGET_TEMPO << 8) | arm9_globalTempo;
    fifoSendValue32(FIFO_TEMPO, word);

    while(1)
    {
        // read keys
        scanKeys();
        if (keysDown() & KEY_A)
        {
            play_stop(module_A);
        }

        if (keysDown() & KEY_B)
        {
            play_stop(module_B);
        }

        if (keysDown() & KEY_UP)
        {
            word = (FIFO_TARGET_BPM << 8) | ((arm9_globalBpm == 255) ? 255 : ++arm9_globalBpm);
            iprintf("BPM:%d - ", arm9_globalBpm);
            if (!fifoSendValue32(FIFO_TEMPO, word))
                iprintf("Could not send...\n");
        }

        if (keysDown() & KEY_DOWN)
        {
            word = (FIFO_TARGET_BPM << 8) | ((arm9_globalBpm == 0) ? 0 : --arm9_globalBpm);
            iprintf("BPM:%d - ", arm9_globalBpm);
            if (!fifoSendValue32(FIFO_TEMPO, word))
                iprintf("Could not send...\n");
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
