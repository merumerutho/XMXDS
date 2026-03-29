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
        iprintf("\x1b[11;1HRoll: %-3d lines %s", arm9_rollN, arm9_rollActive ? "[ON] " : "     ");
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

    // Install FIFO_XMX handlers for ARM7-->ARM9 messages
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
        u32 keys_down = keysDown();
        u32 keys_held = keysHeld();

        // Commands to execute only if module is loaded
        if (MODULE != NULL)
        {
            // MUTE / UNMUTE
            if (keys_held & KEY_TOUCH)
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
            if (keys_down & KEY_A)
                play_stop();

            // TRANSPOSE DOWN
            if (keys_down & KEY_L)
            {
                arm9_globalTranspose--;
                serviceCmd(CMD_SET_TRANSPOSE, arm9_globalTranspose);
            }

            // TRANSPOSE UP
            if (keys_down & KEY_R)
            {
                arm9_globalTranspose++;
                serviceCmd(CMD_SET_TRANSPOSE, arm9_globalTranspose);
            }

            // SET HOT CUE
            if (keys_down & KEY_B)
            {
                arm9_globalHotCuePosition = MODULE->CurrentSongPosition;
                forceUpdate = true;
            }

            // CUE MOVE
            if (keys_held & KEY_B)
            {
                if (keys_down & KEY_LEFT)
                    if (arm9_globalHotCuePosition > 0)
                    {
                        arm9_globalHotCuePosition--;
                        forceUpdate = true;
                    }

                if (keys_down & KEY_RIGHT)
                    if (arm9_globalHotCuePosition < MODULE->ModuleLength - 1)
                    {
                        arm9_globalHotCuePosition++;
                        forceUpdate = true;
                    }
            }

            // LOOP MODE
            if (keys_down & KEY_X)
            {
                arm9_globalLoopMode = !arm9_globalLoopMode;
                serviceCmd(CMD_SET_LOOPMODE, arm9_globalLoopMode);
                forceUpdate = true;
            }

            // BPM LOCK (not while SELECT held, which is reserved for file browser / roll)
            if ((keys_down & KEY_START) && !(keys_held & KEY_SELECT))
            {
                arm9_bpmLock = !arm9_bpmLock;
                serviceCmd(CMD_SET_BPM_LOCK, arm9_bpmLock);
            }

            // GO TO HOT CUED PATTERN AT END OF CURRENT PATTERN
            if (keys_down & KEY_Y)
                serviceCmd(CMD_GOTO_HOTCUE, arm9_globalHotCuePosition);

            // LOOP ROLL (SELECT held as modifier)
            if (keys_held & KEY_SELECT)
            {
                // SELECT+UP: toggle roll on/off with current length
                if (keys_down & KEY_UP)
                {
                    if (!arm9_rollActive)
                    {
                        arm9_rollActive = 1;
                        serviceCmd(CMD_ROLL_START, arm9_rollN);
                    }
                    else
                    {
                        serviceCmd(CMD_ROLL_STOP, 0);
                        arm9_rollActive = 0;
                    }
                }

                // SELECT+RIGHT: double roll length (cap at 128)
                if (keys_down & KEY_RIGHT)
                {
                    if (arm9_rollN < 128) arm9_rollN *= 2;
                    if (arm9_rollActive) serviceCmd(CMD_ROLL_START, arm9_rollN);
                    forceUpdate = true;
                }

                // SELECT+LEFT: halve roll length (floor at 1)
                if (keys_down & KEY_LEFT)
                {
                    if (arm9_rollN > 1) arm9_rollN /= 2;
                    if (arm9_rollActive) serviceCmd(CMD_ROLL_START, arm9_rollN);
                    forceUpdate = true;
                }
            }

            // BPM INCREASE (not while SELECT held for roll)
            if ((keys_down & KEY_UP) && !(keys_held & KEY_SELECT))
            {
                arm9_globalBpm++;
                forceUpdate = true;
            }

            // BPM DECREASE (not while SELECT held for roll)
            if ((keys_down & KEY_DOWN) && !(keys_held & KEY_SELECT))
            {
                arm9_globalBpm--;
                forceUpdate = true;
            }

            // NUDGE FORWARD (not while SELECT or B held)
            if ((keys_down & KEY_RIGHT) && !(keys_held & KEY_B) && !(keys_held & KEY_SELECT))
                nudge = 1;

            // NUDGE BACKWARD (not while SELECT or B held)
            if ((keys_down & KEY_LEFT) && !(keys_held & KEY_B) && !(keys_held & KEY_SELECT))
                nudge = -1;

            // Track any user input
            bAnyUsrInput = (keys_down != 0);

            if ((MODULE->State == XM7_STATE_PLAYING && bAnyUsrInput) || forceUpdate)
                serviceUpdate(nudge);
        }

        // SELECT MODULE (SELECT + START to avoid conflict with roll modifier)
        if ((keys_held & KEY_SELECT) && (keys_down & KEY_START))
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
