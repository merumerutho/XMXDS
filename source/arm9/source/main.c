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
void drawTitle(u32 v);

//---------------------------------------------------------------------------------
void arm9_DebugFIFOHandler(u32 v, void *userdata)
{
    drawTitle(v);
}

void arm9_VBlankHandler()
{

}

void drawIntro()
{
    consoleSelect(&top);
    consoleClear();
    consoleSelect(&bottom);
    iprintf("\x1b[8;13Hxmxds");
    iprintf("\x1b[9;6H{.xm/.mod dj player}");
    iprintf("\x1b[12;10H@merumerutho");
    iprintf("\x1b[13;3Hbased on libxm7 by @sverx");
    while(1)
    {
        scanKeys();
        if (keysDown())
            break;
    }
}

void drawTitle(u32 v)
{
    XM7_ModuleManager_Type* restrict module = deckInfo[0].modManager;
    consoleSelect(&top);
    consoleClear();
    iprintf("\x1b[0;2H _  _ __  __ _  _ ____  ___\n");
    iprintf("\x1b[1;2H( \\/ (  \\/  ( \\/ (  _ \\/ __)\n");
    iprintf("\x1b[2;2H))  ( )    ( )  ( )(_) \\__ \\ \n");
    iprintf("\x1b[3;2H(_/\\_(_/\\/\\_(_/\\_(____/(___/\n");


    iprintf("\x1b[6;0H--------------------------------");

    if (deckInfo[0].modManager != NULL)
    {
        iprintf("\x1b[5;1HBPM: %3d\t\t|\t\tTempo: %2d", arm9_globalBpm, arm9_globalTempo);
        iprintf("\x1b[8;1HSong position:\t%03d/%03d", module->CurrentSongPosition+1, module->ModuleLength);
        iprintf("\x1b[9;1HPattern:\t\t\t%03d", module->CurrentPatternNumber);
        iprintf("\x1b[10;1HLoop:\t\t\t\t%s", module->LoopMode ? "YES" : "NO ");
        iprintf("\x1b[12;1HCue position:\t\t%03d/%03d", arm9_globalCuePosition+1, module->ModuleLength);
        iprintf("\x1b[14;1HTransposition:\t%d  ", module->Transpose);
    }

    if (v!=0)
        iprintf("\x1b[23;1HDebug value: %ld", v);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    XM7_ModuleManager_Type * module = deckInfo[0].modManager;
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
    drawTitle(0);
    drawChannelMatrix();

    irqSet(IRQ_VBLANK, arm9_VBlankHandler);

    bool touchRelease = true;
    while (1)
    {
        scanKeys();

        // Commands to execute only if module is loaded
        if (module != NULL)
        {
            if (keysHeld() & KEY_TOUCH)
            {
                if (touchRelease)
                {
                    touchRead(&touchPos);
                    handleChannelMute(&touchPos);
                    touchRelease = false;
                    // leave this part here! mute won't update otherwise on real hw
                    {
                        fifoGlobalMsg->command = CMD_DUMMY;
                        IpcSend(FIFO_GLOBAL_SETTINGS);
                    }
                }
            }
            else
                touchRelease = true;

            // CUE PLAY
            if(keysDown() & KEY_A)
            {
                if (module != NULL)
                {
                    module->CurrentSongPosition = arm9_globalCuePosition;
                    // If playing, stop
                    if (module->State == XM7_STATE_PLAYING)
                        play_stop(&deckInfo[0]);
                    // Then start
                    play_stop(&deckInfo[0]);
                }
            }

            // PAUSE
            if(keysDown() & KEY_B)
                if (module->State == XM7_STATE_PLAYING)
                    play_stop(&deckInfo[0]);


            // TRANSPOSE UP / DOWN

            if (keysDown() & KEY_L)
            {
                module->Transpose--;
                drawTitle(0);
            }

            if (keysDown() & KEY_R)
            {
                module->Transpose++;
                drawTitle(0);
            }


            // CUE SET
            if(keysDown() & KEY_X)
            {
                arm9_globalCuePosition = module->CurrentSongPosition;
                drawTitle(0);
            }

            // CUE MOVE
            if (keysHeld() & KEY_X)
            {
                if (keysDown() & KEY_LEFT)
                {
                    if (arm9_globalCuePosition > 0)
                        arm9_globalCuePosition--;
                    drawTitle(0);
                }
                if (keysDown() & KEY_RIGHT)
                {
                    if (arm9_globalCuePosition < module->ModuleLength-1)
                        arm9_globalCuePosition++;
                    drawTitle(0);
                }
            }


            // LOOP MODE
            if (keysDown() & KEY_Y)
            {
                module->LoopMode = !(module->LoopMode);
                drawTitle(0);
            }

            // BPM INCREASE
            if (keysDown() & KEY_UP)
            {
                fifoGlobalMsg->data[0] = ++arm9_globalBpm;
                fifoGlobalMsg->data[1] = arm9_globalTempo;
                fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
                IpcSend(FIFO_GLOBAL_SETTINGS);
                consoleSelect(&top);
                iprintf("\x1b[5;1HBPM: %3d", arm9_globalBpm);
            }

            // BPM DECREASE
            if (keysDown() & KEY_DOWN)
            {
                fifoGlobalMsg->data[0] = --arm9_globalBpm;
                fifoGlobalMsg->data[1] = arm9_globalTempo;
                fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
                IpcSend(FIFO_GLOBAL_SETTINGS);
                consoleSelect(&top);
                iprintf("\x1b[5;1HBPM: %3d", arm9_globalBpm);
            }

            // NUDGE FORWARD / BACKWARD
            {
                if ((keysDown() & KEY_RIGHT) && !(keysHeld() & KEY_X))
                    if(module->CurrentTick < module->CurrentTempo)
                        module->CurrentTick++;

                if ((keysDown() & KEY_LEFT) && !(keysHeld() & KEY_X))
                    if(module->CurrentTick > 0)
                        module->CurrentTick--;
            }
        }

        // SELECT MODULE
        if (keysDown() & KEY_SELECT)
        {
            XM7_FS_selectModule((char*) folderPath);
            module = deckInfo[0].modManager;
            fifoGlobalMsg->data[0] = arm9_globalBpm;
            fifoGlobalMsg->data[1] = arm9_globalTempo;
            fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
            IpcSend(FIFO_GLOBAL_SETTINGS);
            drawTitle(0);
            drawChannelMatrix();
        }
        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
