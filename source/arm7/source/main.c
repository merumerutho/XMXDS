#include <nds.h>

#include "arm7_defines.h"
#include "libxm7.h"
#include "arm7_fifo.h"
#include "tempo.h"

// "reserve" FIFO Channel "FIFO_USER_07"
#define FIFO_XM7    (FIFO_USER_07)

//---------------------------------------------------------------------------------
void VblankHandler(void)
{
    //Wifi_Update();
}

//---------------------------------------------------------------------------------
void VcountHandler()
{
    inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB()
{
    exitflag = true;
}

//---------------------------------------------------------------------------------
void XMX_arm7_TimerHandler()
{
    // Execute stuff in the FIFO queue
    while (fifoCheckValue32(FIFO_GLOBAL_SETTINGS))
        arm7_GlobalSettingsFIFOHandler(fifoGetValue32(FIFO_GLOBAL_SETTINGS), NULL);

    // Call libxm7 Timer1Handler (play the module)
    XM7_Timer1Handler();
}

void XMX_arm7_StartPlaying()
{
    // Execute stuff in the FIFO queue (be sure to get CurrentSongPosition information
    while (fifoCheckValue32(FIFO_GLOBAL_SETTINGS))
        arm7_GlobalSettingsFIFOHandler(fifoGetValue32(FIFO_GLOBAL_SETTINGS), NULL);
    // Set current song position to the hot cue
    XM7_Module->CurrentSongPosition = arm7_globalHotCuePosition;
    // Set current song BPM and Tempo to global values
    XM7_Module->CurrentBPM = arm7_globalBpm;
    XM7_Module->CurrentTempo = arm7_globalTempo;

    SetTimerSpeedBPM(arm7_globalBpm);
    XM7_PlayModule(XM7_Module);

    // Override handler with a custom function
    irqSet(IRQ_TIMER1, XMX_arm7_TimerHandler);

    // Re-calculate timer speed
    SetTimerSpeedBPM(arm7_globalBpm);
    fifoSendValue32(FIFO_USER_08, 0); // just to trigger callback
}

void XMX_arm7_StopPlaying()
{
    return XM7_StopModule(XM7_Module);
}

//---------------------------------------------------------------------------------
void XMX_arm7_Value32Handler(u32 p, void *userdata)
{
    if (p != 0)
    {
        // received a pointer to a module that should start now
        XM7_ModuleManager_Type *module = (XM7_ModuleManager_Type*) p;
        XM7_Module = module;
        // Start or stop
        if (module->State == XM7_STATE_PLAYING)
            XMX_arm7_StopPlaying();
        else
            XMX_arm7_StartPlaying();
    }
}

//---------------------------------------------------------------------------------
int main()
{
    // clear sound registers
    dmaFillWords(0, (void*) 0x04000400, 0x100);

    irqInit();
    fifoInit();

    SetYtrigger(80);

    installSoundFIFO();

    // Initialize XM7
    XM7_Initialize();

    // Install the FIFO handler for libXM7 "fifo channel"
    fifoSetValue32Handler(FIFO_XM7, XMX_arm7_Value32Handler, 0);

    installSystemFIFO();

    irqSet(IRQ_VCOUNT, VcountHandler);
    irqSet(IRQ_VBLANK, VblankHandler);

    irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

    setPowerButtonCB(powerButtonCB);

    // Keep the ARM7 mostly idle
    while (!exitflag)
    {
        swiWaitForVBlank();
    }
    return 0;
}
