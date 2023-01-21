#include <nds.h>
#include <stdio.h>

// ARMv9 INCLUDES
#include "arm9_defines.h"
#include "arm9_fifo.h"
#include "nitroFSmenu.h"
#include "play.h"
#include "libXMX.h"
#include "channelMatrix.h"
#include "screens.h"

// ARMv7 INCLUDES
#include "../../arm7/source/libxm7.h"
#include "../../arm7/source/tempo.h"
#include "../../arm7/source/arm7_fifo.h"

#define DEFAULT_ROOT_PATH "./"

//

void arm9_DebugFIFOHandler(u32 p, void *userdata);
void drawTitle();

//---------------------------------------------------------------------------------
void arm9_DebugFIFOHandler(u32 p, void *userdata)
{
    /*for (u8 i = 0; i < 16; i++)
     {
     iprintf("%d", ((IPC_FIFO_packet*) (p))->data[i]);
     }*/
    consoleSelect(&top);
    iprintf("%ld\n", p);
}

void arm9_VBlankHandler()
{

}

void drawIntro()
{
    consoleSelect(&top);
    consoleClear();
    consoleSelect(&bottom);
    iprintf("\x1b[9;13Hxmxds");
    iprintf("\x1b[10;10Hmerumerutho");
    while(1)
    {
        scanKeys();
        if (keysDown())
            break;
    }
}

void drawTitle()
{
    consoleSelect(&top);
    consoleClear();
    iprintf("\x1b[0;2H _  _ __  __ _  _ ____  ___\n");
    iprintf("\x1b[1;2H( \\/ (  \\/  ( \\/ (  _ \\/ __)\n");
    iprintf("\x1b[2;2H))  ( )    ( )  ( )(_) \\__ \\ \n");
    iprintf("\x1b[3;2H(_/\\_(_/\\/\\_(_/\\_(____/(___/\n");

    iprintf("\x1b[5;1HBPM: %3d | Tempo: %2d", arm9_globalBpm, arm9_globalTempo);
    if (deckInfo[0].modManager != NULL)
    {
        iprintf("\x1b[6;1HSong position: %3d\n", deckInfo[0].modManager->CurrentSongPosition);
    }
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    consoleInit(&top, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);
    consoleInit(&bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true);

    drawIntro();

    IpcInit();

    touchPosition touchPos;

    char folderPath[255] = DEFAULT_ROOT_PATH;

    // Install the debugging (for now, only way to print stuff from ARMv7)
    fifoSetValue32Handler(FIFO_USER_08, arm9_DebugFIFOHandler, NULL);
    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);

    XM7_FS_init();
    drawTitle();
    drawChannelMatrix();

    irqSet(IRQ_VBLANK, arm9_VBlankHandler);

    bool touchRelease = true;
    while (1)
    {
        scanKeys();

        if (keysHeld() & KEY_TOUCH)
        {
            if (touchRelease)
            {
                touchRead(&touchPos);
                handleChannelMute(&touchPos);
                touchRelease = false;
            }
        }
        else
            touchRelease = true;

        if (keysDown() & KEY_A)
        {
            if (deckInfo[0].modManager != NULL) play_stop(&deckInfo[0]);
        }


        if (keysDown() & KEY_UP)
        {
            fifoGlobalMsg->data[0] = ++arm9_globalBpm;
            fifoGlobalMsg->data[1] = arm9_globalTempo;
            fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
            IpcSend(FIFO_GLOBAL_SETTINGS);
            consoleSelect(&top);
            iprintf("\x1b[5;1HBPM: %3d", arm9_globalBpm);
        }

        if (keysDown() & KEY_DOWN)
        {
            fifoGlobalMsg->data[0] = --arm9_globalBpm;
            fifoGlobalMsg->data[1] = arm9_globalTempo;
            fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
            IpcSend(FIFO_GLOBAL_SETTINGS);
            consoleSelect(&top);
            iprintf("\x1b[5;1HBPM: %3d", arm9_globalBpm);
        }

        if (keysDown() & KEY_SELECT)
        {
            XM7_FS_selectModule((char*) folderPath);
            fifoGlobalMsg->data[0] = arm9_globalBpm;
            fifoGlobalMsg->data[1] = arm9_globalTempo;
            fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
            IpcSend(FIFO_GLOBAL_SETTINGS);
            drawTitle();
            drawChannelMatrix();
        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
