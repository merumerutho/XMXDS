#include <nds.h>
#include <stdio.h>
#include <math.h>

// ARMv9 INCLUDES
#include "arm9_defines.h"
#include "nitroFSmenu.h"
#include "play.h"
#include "libXMX.h"

// ARMv7 INCLUDES
#include "../../arm7/source/libxm7.h"
#include "../../arm7/source/tempo.h"
#include "../../arm7/source/arm7_fifo.h"

#define DEFAULT_ROOT_PATH "./"

u8 arm9_globalBpm = DEFAULT_BPM;
u8 arm9_globalTempo = DEFAULT_TEMPO;

//

void arm9_DebugFIFOHandler(u32 p, void *userdata);
void drawIntro();
void drawVolumeFader();
void evaluateVolumeFadersAndCrossFader(touchPosition touchPos);
void IpcSend(IPC_FIFO_packet *pkt, u8 fifo);
void sendBpmTempo(IPC_FIFO_packet *ipc_packet, u8 bpm, u8 tempo);

//---------------------------------------------------------------------------------
void arm9_DebugFIFOHandler(u32 p, void *userdata)
{
    /*for (u8 i = 0; i < 16; i++)
     {
     iprintf("%d", ((IPC_FIFO_packet*) (p))->data[i]);
     }*/
    iprintf("%ld\n", p);
}

void drawIntro()
{
    consoleClear();
    iprintf("\x1b[0;0H-------------------------------");
    for (u8 i = 0; i < 24; i++)
    {
        iprintf("\x1b[%d;0H-", i);
        iprintf("\x1b[%d;31H-", i);
    }
    iprintf("\x1b[23;0H-------------------------------");

    iprintf("\x1b[2;6H.oO:Oo. XMXDS .oO:Oo.\n\n");
    iprintf("\x1b[3;1H2-decks module player for NDS!\n");
    iprintf("\x1b[4;4Hcredits: @merumerutho\n");
    iprintf("\x1b[5;4Hbased on libxm7 (@sverx)\n\n");
    drawVolumeFader();
}

void drawVolumeFader()
{
    // Draw position of volume bar on screen
    for (u8 i=0; i < 13; i++)
    {
        if (i==loadedModulesInfo[0].modManager->CurrentGlobalVolume/10)
            iprintf("\x1b[%d;2H*", 20 - i);
        else
            iprintf("\x1b[%d;2H|", 20 - i);
        if (i==loadedModulesInfo[1].modManager->CurrentGlobalVolume/10)
            iprintf("\x1b[%d;28H*", 20 - i);
        else
            iprintf("\x1b[%d;28H|", 20 - i);
    }
    iprintf("\x1b[21;1H%3d", loadedModulesInfo[0].modManager->CurrentGlobalVolume);
    iprintf("\x1b[21;27H%3d", loadedModulesInfo[1].modManager->CurrentGlobalVolume);
}

void evaluateVolumeFadersAndCrossFader(touchPosition touchPos)
{
    u8 targetModule = 0xFF;
    u16 px = ((touchPos.rawx - 320) * SCREEN_WIDTH) / 3488;
    u16 py = SCREEN_HEIGHT - ((touchPos.rawy - 224) * SCREEN_HEIGHT) / 3680;

    if (abs(px - 20) < 10) targetModule = 0;
    if (abs(px - (SCREEN_WIDTH - 20)) < 10) targetModule = 1;

    u16 value = (u16) (MIN(MAX(0, (py-30)*1.7), 170) * 127 / 170);
    if(targetModule < 2)
        loadedModulesInfo[targetModule].modManager->CurrentGlobalVolume = value;

}

void IpcSend(IPC_FIFO_packet *pkt, u8 fifo)
{
    fifoSendValue32(fifo, (u32) pkt);
}

void sendBpmTempo(IPC_FIFO_packet *ipc_packet, u8 bpm, u8 tempo)
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

    touchPosition touchPos;
    IPC_FIFO_packet *ipc_packet = malloc(sizeof(IPC_FIFO_packet));

    char folderPath[255] = DEFAULT_ROOT_PATH;

    // Install the debugging (for now, only way to print stuff from ARMv7)
    fifoSetValue32Handler(FIFO_USER_08, arm9_DebugFIFOHandler, NULL);
    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);
    // Send BPM/TEMPO to ARMv7
    sendBpmTempo(ipc_packet, arm9_globalBpm, arm9_globalTempo);

    XM7_FS_init();
    drawIntro();

    while (1)
    {
        scanKeys();
        if(keysHeld() & KEY_TOUCH)
        {
            touchRead(&touchPos);
            evaluateVolumeFadersAndCrossFader(touchPos);
            drawVolumeFader();
        }

        if ((keysHeld() & KEY_L) && (keysUp() & KEY_A))
        {
            if (loadedModulesInfo[0].modManager != NULL) play_stop(&loadedModulesInfo[0]);
        }

        if ((keysHeld() & KEY_R) && (keysUp() & KEY_A))
        {
            if (loadedModulesInfo[1].modManager != NULL) play_stop(&loadedModulesInfo[1]);
        }

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
            XM7_FS_selectModule((char*) folderPath);
            IpcSend(ipc_packet, FIFO_GLOBAL_SETTINGS);
            drawIntro();
        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
