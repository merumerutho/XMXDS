#include <nds.h>
#include <stdio.h>

// ARMv9 INCLUDES
#include "arm9_defines.h"
#include "arm9_fifo.h"
#include "filesystem.h"
#include "play.h"
#include "libXMX.h"
#include "channelMatrix.h"
#include "screens.h"

// ARMv7 INCLUDES
#include "libxm7.h"
#include "../../arm7/source/tempo.h"
#include "../../arm7/source/arm7_fifo.h"


#define DEFAULT_ROOT_PATH "./"

#define MODULE (deckInfo.modManager)

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

void updateArmV7(int32_t nudge)
{
    fifoGlobalMsg->data[0] = arm9_globalBpm;
    fifoGlobalMsg->data[1] = arm9_globalTempo;
    fifoGlobalMsg->data[2] = arm9_globalHotCuePosition;
    fifoGlobalMsg->data[3] = nudge;
    fifoGlobalMsg->command = CMD_SET_GLOBAL_SETTINGS;
    IpcSend(FIFO_XMX);
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
    consoleSelect(&top);
    consoleClear();
    iprintf("\x1b[0;2H _  _ __  __ _  _ ____  ___\n");
    iprintf("\x1b[1;2H( \\/ (  \\/  ( \\/ (  _ \\/ __)\n");
    iprintf("\x1b[2;2H))  ( )    ( )  ( )(_) \\__ \\ \n");
    iprintf("\x1b[3;2H(_/\\_(_/\\/\\_(_/\\_(____/(___/\n");
    iprintf("\x1b[4;0H--------------------------------");
    iprintf("\x1b[6;0H--------------------------------");

    if (MODULE != NULL)
    {
        iprintf("\x1b[5;1HBPM:\t\t\t%3d  Tempo:\t\t%2d", arm9_globalBpm, arm9_globalTempo);
        iprintf("\x1b[8;1HSong position:\t%03d/%03d", MODULE->CurrentSongPosition + 1, MODULE->ModuleLength);
        iprintf("\x1b[9;1HHotCue position:\t%03d/%03d", arm9_globalHotCuePosition + 1, MODULE->ModuleLength);
        iprintf("\x1b[10;1HPtn. Loop:\t\t\t%s", MODULE->LoopMode ? "YES" : "NO ");

        iprintf("\x1b[12;1HNote position:\t%03d/%03d", MODULE->CurrentLine, MODULE->PatternLength[MODULE->CurrentPatternNumber]);

        iprintf("\x1b[14;1HTransposition:\t%d  ", MODULE->Transpose);
    }
    if (v != 0) iprintf("\x1b[23;1HDebug value: %ld", v);
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    touchPosition touchPos;
    char folderPath[255] = DEFAULT_ROOT_PATH;

    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    IpcInit();

    // Install the debugging (for now, only way to print stuff from ARMv7)
    fifoSetValue32Handler(FIFO_XMX, arm9_DebugFIFOHandler, NULL);

    // Initialize two consoles (top and bottom)
    consoleInit(&top, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, true, true);
    consoleInit(&bottom, 0, BgType_Text4bpp, BgSize_T_256x256, 2, 0, false, true);
    drawIntro();

    // turn on master sound
    fifoSendValue32(FIFO_SOUND, SOUND_MASTER_ENABLE);

    // Initialize filesystem
    XMX_FileSystem_init();

    // Draw bottom screen
    drawChannelMatrix();

    bool inputTouching = false;

    while (TRUE)
    {
        int nudge = 0;
        drawTitle(0);
        scanKeys();

        // Commands to execute only if module is loaded
        if (MODULE != NULL)
        {
            // MUTE / UNMUTE
            if (keysHeld() & KEY_TOUCH)
            {
                if (!inputTouching)
                {
                    touchRead(&touchPos);
                    handleChannelMute(&touchPos);
                    inputTouching = true;
                }
            }
            else
                inputTouching = false;

            // CUE PLAY
            if (keysDown() & KEY_A)
                    play_stop();

            // TRANSPOSE DOWN
            if (keysDown() & KEY_L)
                MODULE->Transpose--;

            // TRANSPOSE UP
            if (keysDown() & KEY_R)
                MODULE->Transpose++;

            // SET HOT CUE
            if (keysDown() & KEY_B)
                arm9_globalHotCuePosition = MODULE->CurrentSongPosition;

            // CUE MOVE
            if (keysHeld() & KEY_B)
            {
                if (keysDown() & KEY_LEFT)
                    if (arm9_globalHotCuePosition > 0)
                        arm9_globalHotCuePosition--;

                if (keysDown() & KEY_RIGHT)
                    if (arm9_globalHotCuePosition < MODULE->ModuleLength - 1)
                        arm9_globalHotCuePosition++;
            }

            // LOOP MODE
            if (keysDown() & KEY_X)
                MODULE->LoopMode = !(MODULE->LoopMode);

            // GO TO HOT CUEd PATTERN AT END OF CURRENT PATTERN
            if (keysDown() & KEY_Y)
            {
                // Only necessary to set CurrentSongPosition.
                // It will be evaluated only at end of pattern by design
                MODULE->bGotoHotCue = TRUE;
                MODULE->CurrentSongPosition = arm9_globalHotCuePosition;
            }

            // BPM INCREASE
            if (keysDown() & KEY_UP)
                arm9_globalBpm++;

            // BPM DECREASE
            if (keysDown() & KEY_DOWN)
                arm9_globalBpm--;

            // NUDGE FORWARD
            if ((keysDown() & KEY_RIGHT) && !(keysHeld() & KEY_B))
                nudge = 1;

            // NUDGE BACKWARD
            if ((keysDown() & KEY_LEFT) && !(keysHeld() & KEY_B))
                nudge = -1;

            if (MODULE->State == XM7_STATE_PLAYING)
                updateArmV7(nudge);  // This is used to pass changes to armv7
        }

        // SELECT MODULE
        if (keysDown() & KEY_SELECT)
        {
            XMX_FileSystem_selectModule((char*) folderPath);
            // After function ends, re-draw bottom screen
            drawChannelMatrix();
            // Update armv7
            updateArmV7(0);
        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
