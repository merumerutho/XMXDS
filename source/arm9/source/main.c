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

#define THE_MODULE (deckInfo[0].modManager)

//

void arm9_DebugFIFOHandler(u32 p, void *userdata);
void drawTitle(u32 v);

//---------------------------------------------------------------------------------
void arm9_DebugFIFOHandler(u32 v, void *userdata)
{

}

void arm9_VBlankHandler()
{

}

void updateArmV7()
{
    fifoGlobalMsg->data[0] = arm9_globalBpm;
    fifoGlobalMsg->data[1] = arm9_globalTempo;
    fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
    IpcSend(FIFO_GLOBAL_SETTINGS);
    //drawTitle(0);
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
    while (1)
    {
        scanKeys();
        if (keysDown()) break;
    }
}

void drawTitle(u32 v)
{
    XM7_ModuleManager_Type *module = deckInfo[0].modManager;
    consoleSelect(&top);
    consoleClear();
    iprintf("\x1b[0;2H _  _ __  __ _  _ ____  ___\n");
    iprintf("\x1b[1;2H( \\/ (  \\/  ( \\/ (  _ \\/ __)\n");
    iprintf("\x1b[2;2H))  ( )    ( )  ( )(_) \\__ \\ \n");
    iprintf("\x1b[3;2H(_/\\_(_/\\/\\_(_/\\_(____/(___/\n");
    iprintf("\x1b[6;0H--------------------------------");

    if (module != NULL)
    {
        iprintf("\x1b[5;1HBPM: %3d\t\t|\t\tTempo: %2d", arm9_globalBpm, arm9_globalTempo);
        iprintf("\x1b[8;1HSong position:\t%03d/%03d", module->CurrentSongPosition + 1, module->ModuleLength);
        iprintf("\x1b[9;1HPattern:\t\t\t%03d", module->CurrentPatternNumber);
        iprintf("\x1b[10;1HLoop:\t\t\t\t%s", module->LoopMode ? "YES" : "NO ");
        iprintf("\x1b[12;1HHot Cue position:\t%03d/%03d", arm9_globalHotCuePosition + 1, module->ModuleLength);
        iprintf("\x1b[14;1HTransposition:\t%d  ", module->Transpose);
    }
    if (v != 0) iprintf("\x1b[23;1HDebug value: %ld", v);
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
    drawTitle(0);
    drawChannelMatrix();

    irqSet(IRQ_VBLANK, arm9_VBlankHandler);

    bool touchRelease = true;
    while (1)
    {
        drawTitle(0);
        scanKeys();

        // Commands to execute only if module is loaded
        if (THE_MODULE != NULL)
        {
            if (keysHeld() & KEY_TOUCH)
            {
                if (touchRelease)
                {
                    touchRead(&touchPos);
                    handleChannelMute(&touchPos);
                    touchRelease = false;
                    updateArmV7();
                }
            }
            else
                touchRelease = true;

            // CUE PLAY
            if (keysDown() & KEY_A)
            {
                THE_MODULE->CurrentSongPosition = arm9_globalHotCuePosition;
                // If playing, stop
                if (THE_MODULE->State == XM7_STATE_PLAYING) play_stop(&deckInfo[0]);
                // Then start
                play_stop(&deckInfo[0]);
                updateArmV7();  // This can be used to 'notify' armv7 of changes
            }

            // PAUSE
            if (keysDown() & KEY_B)
            {
                if (THE_MODULE->State == XM7_STATE_PLAYING)
                {
                    play_stop(&deckInfo[0]);
                    THE_MODULE->State = XM7_STATE_STOPPED;
                    updateArmV7();  // This can be used to 'notify' armv7 of changes
                }
            }

            // TRANSPOSE DOWN
            if (keysDown() & KEY_L)
            {
                THE_MODULE->Transpose--;
                updateArmV7();  // This can be used to 'notify' armv7 of changes
            }

            // TRANSPOSE UP
            if (keysDown() & KEY_R)
            {
                THE_MODULE->Transpose++;
                updateArmV7();  // This can be used to 'notify' armv7 of changes
            }

            // SET HOT CUE
            if (keysDown() & KEY_Y)
            {
                arm9_globalHotCuePosition = THE_MODULE->CurrentSongPosition;
                updateArmV7();  // This can be used to 'notify' armv7 of changes
            }

            // CUE MOVE
            if (keysHeld() & KEY_Y)
            {
                if (keysDown() & KEY_LEFT)
                {
                    if (arm9_globalHotCuePosition > 0) arm9_globalHotCuePosition--;
                    updateArmV7();
                }
                if (keysDown() & KEY_RIGHT)
                {
                    if (arm9_globalHotCuePosition < THE_MODULE->ModuleLength - 1) arm9_globalHotCuePosition++;
                    updateArmV7();
                }
            }

            // LOOP MODE
            if (keysDown() & KEY_X)
            {
                THE_MODULE->LoopMode = !(THE_MODULE->LoopMode);
                updateArmV7();  // This can be used to update arm7 of some changes
            }

            // BPM INCREASE
            if (keysDown() & KEY_UP)
            {
                arm9_globalBpm++;
                updateArmV7();
            }

            // BPM DECREASE
            if (keysDown() & KEY_DOWN)
            {
                arm9_globalBpm--;
                updateArmV7();
            }

            // NUDGE FORWARD
            if ((keysDown() & KEY_RIGHT) && !(keysHeld() & KEY_Y))
            {
                if (THE_MODULE->CurrentTick < THE_MODULE->CurrentTempo)
                {
                    THE_MODULE->CurrentTick++;
                }
                else if (THE_MODULE->CurrentLine < THE_MODULE->PatternLength[THE_MODULE->CurrentPatternNumber])
                {
                    THE_MODULE->CurrentTick = 0;
                    THE_MODULE->CurrentLine++;
                }
                updateArmV7();
            }
            // NUDGE BACKWARD
            if ((keysDown() & KEY_LEFT) && !(keysHeld() & KEY_Y))
            {
                if (THE_MODULE->CurrentTick > 0)
                {
                    THE_MODULE->CurrentTick--;
                }
                else if (THE_MODULE->CurrentLine > 0)
                {
                    THE_MODULE->CurrentTick = THE_MODULE->CurrentTempo - 1;
                    THE_MODULE->CurrentLine--;
                }
                updateArmV7();
            }
        }

        // SELECT MODULE
        if (keysDown() & KEY_SELECT)
        {
            if (XM7_FS_selectModule((char*) folderPath) == 0)
            {
                // Module loaded
                arm9_globalHotCuePosition = 0;  // reset this value to 0
            }
            drawChannelMatrix();
            updateArmV7();
        }

        // Idk why it seems to be needed
        //if (keysDown() & KEY_START)
        //    doNothing();

        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
