#include <nds.h>
#include <stdio.h>

// ARMv9 INCLUDES
#include "arm9_defines.h"
#include "nitroFSmenu.h"
#include "play.h"
#include "libXMX.h"
#include "screens.h"

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
void IpcSend(FifoMsg *pkt, u8 fifo);
void sendBpmTempo(FifoMsg *ipc_packet, u8 bpm, u8 tempo);

//---------------------------------------------------------------------------------
void arm9_DebugFIFOHandler(u32 p, void *userdata)
{
    /*for (u8 i = 0; i < 16; i++)
     {
     iprintf("%d", ((IPC_FIFO_packet*) (p))->data[i]);
     }*/
    iprintf("%ld\n", p);
}

void arm9_VBlankHandler()
{

}

void drawIntro()
{
    consoleSelect(&top);
    consoleClear();
    iprintf("XMXDS!");
    consoleSelect(&bottom);
    consoleClear();
    iprintf("\x1b[0;0H-------------------------------");
    for (u8 i = 0; i < 24; i++)
    {
        iprintf("\x1b[%d;0H-", i);
        iprintf("\x1b[%d;31H-", i);
    }
    iprintf("\x1b[23;0H-------------------------------");
}

void IpcSend(FifoMsg *pkt, u8 fifo)
{
    fifoSendValue32(fifo, (u32) pkt);
}

void sendBpmTempo(FifoMsg *ipc_packet, u8 bpm, u8 tempo)
{
    ipc_packet->data[0] = bpm;
    ipc_packet->data[1] = tempo;
    ipc_packet->command = CMD_SET_BPM_TEMPO;
    IpcSend(ipc_packet, FIFO_GLOBAL_SETTINGS);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    consoleInit(&top, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);
    consoleInit(&bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true);

    touchPosition touchPos;
    FifoMsg *fifo_msg = malloc(sizeof(FifoMsg));

    char folderPath[255] = DEFAULT_ROOT_PATH;

    // Install the debugging (for now, only way to print stuff from ARMv7)
    fifoSetValue32Handler(FIFO_USER_08, arm9_DebugFIFOHandler, NULL);
    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);
    // Send BPM/TEMPO to ARMv7
    sendBpmTempo(fifo_msg, arm9_globalBpm, arm9_globalTempo);

    XM7_FS_init();
    drawIntro();

    irqSet(IRQ_VBLANK, arm9_VBlankHandler);

    while (1)
    {
        scanKeys();

        if (keysHeld() & KEY_TOUCH)
        {
            touchRead(&touchPos);
            // Do nothing for now
        }

        if (keysDown() & KEY_A)
        {
            if (deckInfo[0].modManager != NULL) play_stop(&deckInfo[0]);
        }


        if (keysDown() & KEY_UP)
        {
            fifo_msg->data[0] = ++arm9_globalBpm;
            fifo_msg->data[1] = arm9_globalTempo;
            fifo_msg->command = CMD_SET_BPM_TEMPO;
            IpcSend(fifo_msg, FIFO_GLOBAL_SETTINGS);
            iprintf("bpm: %d - ", arm9_globalBpm);
        }

        if (keysDown() & KEY_DOWN)
        {
            fifo_msg->data[0] = --arm9_globalBpm;
            fifo_msg->data[1] = arm9_globalTempo;
            fifo_msg->command = CMD_SET_BPM_TEMPO;
            IpcSend(fifo_msg, FIFO_GLOBAL_SETTINGS);
            iprintf("bpm: %d - ", arm9_globalBpm);
        }

        if (keysDown() & KEY_SELECT)
        {
            XM7_FS_selectModule((char*) folderPath);
            IpcSend(fifo_msg, FIFO_GLOBAL_SETTINGS);
            drawIntro();
        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
