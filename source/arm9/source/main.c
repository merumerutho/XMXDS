#include <nds.h>
#include <stdio.h>

// ARMv9 INCLUDES
#include "arm9_defines.h"
#include "nitroFSmenu.h"

// ARMv7 INCLUDES
#include "../../arm7/source/libxm7.h"
#include "../../arm7/source/tempo.h"
#include "../../arm7/source/arm7_fifo.h"

#define DEFAULT_ROOT_PATH "./"

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
    IPC_FIFO_packet* ipc_packet = malloc(sizeof(IPC_FIFO_packet));

    XM7_ModuleManager_Type* modules[LIBXM7_ALLOWED_MODULES];
    for (u8 i=0; i < LIBXM7_ALLOWED_MODULES; i++)
        modules[i] = malloc(sizeof(XM7_ModuleManager_Type));

    char folderPath[255] = DEFAULT_ROOT_PATH;

	// Install the debugging (for now, only way to print stuff from ARMv7)
    fifoSetValue32Handler(FIFO_USER_08, arm9_DebugFIFOHandler, NULL);
    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);
    // Send BPM/TEMPO to ARMv7
    sendBpmTempo(ipc_packet, arm9_globalBpm, arm9_globalTempo);

    XM7_FS_init();
	displayIntro();

    while(1)
    {
        scanKeys();

        if (keysUp() & KEY_A)
            play_stop(modules[0]);

        if (keysUp() & KEY_B)
            play_stop(modules[1]);

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

        if (keysDown() & KEY_SELECT)
        {
            u8 idx = XM7_FS_selectModule((char *) folderPath, modules);
            displayIntro();
            if(idx!=0xFF)
                iprintf("Loaded %s on slot %c\n", modules[idx]->ModuleName, idx?'B':'A');

        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
	return 0;
}
