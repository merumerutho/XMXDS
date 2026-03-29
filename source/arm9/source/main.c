#include <nds.h>
#include <stdio.h>

#include "arm9_defines.h"
#include "arm9_fifo.h"
#include "filesystem.h"
#include "play.h"
#include "libXMX.h"
#include "channelMatrix.h"
#include "screens.h"
#include "libxm7.h"

#define DEFAULT_ROOT_PATH "./"

#define MODULE (deckInfo.modManager)

void drawTitle();

//---------------------------------------------------------------------------------
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
    while (1)
    {
        scanKeys();
        if (keysDown()) break;
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

    if (MODULE != NULL)
    {
        // Invalidate ARM9 cache so we read ARM7's latest writes from main RAM
        DC_InvalidateRange(MODULE, sizeof(XM7_ModuleManager_Type));
        iprintf("\x1b[4;0H--------------------------------");
        iprintf("\x1b[6;0H--------------------------------");
        iprintf("\x1b[5;1HBPM:\t\t\t%3d  Tempo:\t\t%2d", MODULE->CurrentBPM, MODULE->CurrentTempo);
        iprintf("\x1b[8;1HSong position:\t%03d/%03d", MODULE->CurrentSongPosition + 1, MODULE->ModuleLength);
        iprintf("\x1b[9;1HHotCue position:\t%03d/%03d", arm9_globalHotCuePosition + 1, MODULE->ModuleLength);

        // LoopMode, Transpose, BPM lock read from ARM9 shadow
        iprintf("\x1b[7;1HBPM Lock: %-3s   %s",
                arm9_bpmLock ? "ON" : "OFF",
                arm9_beatCounter > 0 ? "*" : " ");
        if (arm9_beatCounter > 0) arm9_beatCounter--;

        iprintf("\x1b[10;1HPtn. Loop:\t\t\t%s", arm9_globalLoopMode ? "YES" : "NO ");
        iprintf("\x1b[12;1HNote position:\t%03d/%03d", MODULE->CurrentLine, MODULE->PatternLength[MODULE->CurrentPatternNumber]);
        iprintf("\x1b[14;1HTransposition:\t%d  ", arm9_globalTranspose);
    }
}

//---------------------------------------------------------------------------------
int main(int argc, char **argv)
{
    touchPosition touchPos;
    char folderPath[255] = DEFAULT_ROOT_PATH;

    videoSetMode(MODE_0_2D);
    videoSetModeSub(MODE_0_2D);

    // Install FIFO_XMX handlers for ARM7→ARM9 messages
    fifoSetAddressHandler(FIFO_XMX, arm9_XMXServiceHandler, NULL);
    fifoSetValue32Handler(FIFO_XMX, arm9_XMXValueHandler, NULL);

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
        bool forceUpdate = false;
        bool bAnyUsrInput = false;
        int nudge = 0;
        drawTitle();
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
            {
                inputTouching = false;
            }

            // CUE PLAY
            if (keysDown() & KEY_A)
                play_stop();

            // TRANSPOSE DOWN
            if (keysDown() & KEY_L)
            {
                arm9_globalTranspose--;
                serviceCmd(CMD_SET_TRANSPOSE, arm9_globalTranspose);
            }

            // TRANSPOSE UP
            if (keysDown() & KEY_R)
            {
                arm9_globalTranspose++;
                serviceCmd(CMD_SET_TRANSPOSE, arm9_globalTranspose);
            }

            // SET HOT CUE
            if (keysDown() & KEY_B)
            {
                arm9_globalHotCuePosition = MODULE->CurrentSongPosition;
                forceUpdate = true;
            }

            // CUE MOVE
            if (keysHeld() & KEY_B)
            {
                if (keysDown() & KEY_LEFT)
                    if (arm9_globalHotCuePosition > 0)
                    {
                        arm9_globalHotCuePosition--;
                        forceUpdate = true;
                    }

                if (keysDown() & KEY_RIGHT)
                    if (arm9_globalHotCuePosition < MODULE->ModuleLength - 1)
                    {
                        arm9_globalHotCuePosition++;
                        forceUpdate = true;
                    }
            }

            // LOOP MODE
            if (keysDown() & KEY_X)
            {
                arm9_globalLoopMode = !arm9_globalLoopMode;
                serviceCmd(CMD_SET_LOOPMODE, arm9_globalLoopMode);
                forceUpdate = true;
            }

            // BPM LOCK
            if (keysDown() & KEY_START)
            {
                arm9_bpmLock = !arm9_bpmLock;
                serviceCmd(CMD_SET_BPM_LOCK, arm9_bpmLock);
            }

            // GO TO HOT CUED PATTERN AT END OF CURRENT PATTERN
            if (keysDown() & KEY_Y)
                serviceCmd(CMD_GOTO_HOTCUE, arm9_globalHotCuePosition);

            // BPM INCREASE
            if (keysDown() & KEY_UP)
            {
                arm9_globalBpm++;
                forceUpdate = true;
            }

            // BPM DECREASE
            if (keysDown() & KEY_DOWN)
            {
                arm9_globalBpm--;
                forceUpdate = true;
            }

            // NUDGE FORWARD
            if ((keysDown() & KEY_RIGHT) && !(keysHeld() & KEY_B))
                nudge = 1;

            // NUDGE BACKWARD
            if ((keysDown() & KEY_LEFT) && !(keysHeld() & KEY_B))
                nudge = -1;

            // Track any user input
            bAnyUsrInput = (keysDown() != 0);

            if ((MODULE->State == XM7_STATE_PLAYING && bAnyUsrInput) || forceUpdate)
                serviceUpdate(nudge);
        }

        // SELECT MODULE
        if (keysDown() & KEY_SELECT)
        {
            XMX_FileSystem_selectModule((char*) folderPath);
            // After function ends, re-draw bottom screen
            drawChannelMatrix();
            // Update ARM7 with current params
            serviceUpdate(0);
        }

        // Wait for VBlank
        swiWaitForVBlank();
    };
    return 0;
}
