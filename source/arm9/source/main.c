#include <nds.h>
#include <stdio.h>

// ARMv9 INCLUDES
#include "arm9_defines.h"
#include "nitroFSmenu.h"

// ARMv7 INCLUDES
#include "../../arm7/source/libxm7.h"
#include "../../arm7/source/tempo.h"
#include "../../arm7/source/arm7_fifo.h"

#define MOD_TO_PLAY_A "data/mods/myFolder/demo_D.xm"
#define MOD_TO_PLAY_B "data/mods/myFolder/demo_E.xm"


u8 arm9_globalBpm = DEFAULT_BPM;
u8 arm9_globalTempo = DEFAULT_TEMPO;

bool playingA;
bool playingB;

//---------------------------------------------------------------------------------
void arm9_DebugFIFOHandler (u32 p, void* userdata)
{
    for(u8 i=0; i<16; i++)
    {
        iprintf("%c", (char)((IPC_FIFO_packet*)(p))->data[0] );
    }
    iprintf("\n");
}

void displayIntro()
{
    iprintf(" \t\t.oO:Oo. XMXDS .oO:Oo.\n\n");
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

void IpcSend(IPC_FIFO_packet* pkt, u8 fifo)
{
    fifoSendValue32(fifo, (u32) pkt);
}

void sendBpmTempo(IPC_FIFO_packet* ipc_packet, u8 bpm, u8 tempo)
{
    ipc_packet->data[0] = bpm;
    ipc_packet->data[1] = tempo;
    ipc_packet->command = CMD_SET_BPM_TEMPO;
    IpcSend(ipc_packet, FIFO_GLOBAL_SETTINGS);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    consoleDemoInit();

    XM7_ModuleManager_Type* module_A = NULL;
    XM7_ModuleManager_Type* module_B = NULL;

    FILE* modA_file;
    FILE* modB_file;

    long fszA = 0, fszB = 0;

    char folderPath[255] = "data/mods/";

    IPC_FIFO_packet* ipc_packet = malloc(sizeof(IPC_FIFO_packet));

    // Load nitroFS file system
	if(!nitroFSInit(NULL))
        iprintf("Could not load nitroFS!\n");

	// Install the debugging (for now, only way to print stuff from ARMv7)
    fifoSetValue32Handler(FIFO_USER_08, arm9_DebugFIFOHandler, NULL);

	displayIntro();

    // File reading check
    modA_file = fopen(MOD_TO_PLAY_A, "rb");
    fseek(modA_file, 0L, SEEK_END);
    fszA = ftell(modA_file);
    rewind(modA_file);

    modB_file = fopen(MOD_TO_PLAY_B, "rb");
    fseek(modB_file, 0L, SEEK_END);
    fszB = ftell(modB_file);
    rewind(modB_file);

    // Prepare destination pointer
    u8* modA_data = malloc(sizeof(u8) * (fszA));
    u8* modB_data = malloc(sizeof(u8) * (fszB));

    // Read XM data from file pointer
    if(fread(modA_data, sizeof(u8), fszA, modA_file) != fszA)
        printf("\tCould not read module A correctly!\n");

    if(fread(modB_data, sizeof(u8), fszB, modB_file) != fszB)
            printf("\tCould not read module B correctly!\n");

    if (fszA > 0 && fszB > 0)
    {
        // allocate memory for the module
        module_A = malloc(sizeof(XM7_ModuleManager_Type));
        module_B = malloc(sizeof(XM7_ModuleManager_Type));
        // Load XM
        XM7_LoadXM(module_A, (XM7_XMModuleHeader_Type*) modA_data, 0);
        XM7_LoadXM(module_B, (XM7_XMModuleHeader_Type*) modB_data, 1);

        // ensure data gets written to main RAM
        DC_FlushAll();
    }

    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);

    // Send BPM/TEMPO to ARMv7
    sendBpmTempo(ipc_packet, arm9_globalBpm, arm9_globalTempo);

    XM7_FS_selectModule((char *) folderPath);

    while(1)
    {
        scanKeys();

        if (keysUp() & KEY_A)
            play_stop(module_A);

        if (keysUp() & KEY_B)
            play_stop(module_B);

        if (keysDown() & KEY_UP)
        {
            ipc_packet->data[0] = ++arm9_globalBpm;
            ipc_packet->data[1] = arm9_globalTempo;
            ipc_packet->command = CMD_SET_BPM_TEMPO;
            IpcSend(ipc_packet, FIFO_GLOBAL_SETTINGS);
            iprintf("bpm: %d - ", arm9_globalBpm);
        }

        if (keysDown() & KEY_DOWN)
        {
            ipc_packet->data[0] = --arm9_globalBpm;
            ipc_packet->data[1] = arm9_globalTempo;
            ipc_packet->command = CMD_SET_BPM_TEMPO;
            IpcSend(ipc_packet, FIFO_GLOBAL_SETTINGS);
            iprintf("bpm: %d - ", arm9_globalBpm);
        }
        // Wait for VBlank
        swiWaitForVBlank();
    };
	return 0;
}
