/*
 * XMXPlayer.c
 * These function wrap the libxm7 library
 */

#include "XMXPlayer.h"

#include "arm7_defines.h"
#include "arm7_fifo.h"
#include "libxm7.h"
#include "tempo.h"

//---------------------------------------------------------------------------------

void XMX_Initialize()
{
    // Initialize XM7
    XM7_Initialize();
}

//---------------------------------------------------------------------------------

void XMXPlayer_arm7_TimerHandler()
{
    // Execute everything in the FIFO queue
    while (fifoCheckAddress(FIFO_XMX))
    {
        arm7_XMXServiceHandler((XMXServiceMsg_S*) fifoGetAddress(FIFO_XMX), NULL);
    }

    // Call libxm7 Timer1Handler
    XM7_Timer1Handler();
}

//---------------------------------------------------------------------------------

void XMXPlayer_arm7_StartPlaying()
{
    // Execute stuff in the FIFO queue (be sure to get CurrentSongPosition information)
    while (fifoCheckAddress(FIFO_XMX))
    {
        arm7_XMXServiceHandler((XMXServiceMsg_S*) fifoGetAddress(FIFO_XMX), NULL);
    }

    // Set current song position to the hot cue
    XM7_Module->CurrentSongPosition = arm7_globalHotCuePosition;

    // Set current song BPM and Tempo to global values
    XM7_Module->CurrentBPM = arm7_globalBpm;
    XM7_Module->CurrentTempo = XM7_Module->DefaultTempo;

    // Set Timer to match the BPM
    SetTimerSpeedBPM(arm7_globalBpm);

    // Start playing (here the timer callback associated with a libxm7 function)
    XM7_PlayModule(XM7_Module);

    // Immediately override the timer callback with a custom function
    irqSet(IRQ_TIMER1, XMXPlayer_arm7_TimerHandler);
    
    // Re-calculate timer speed
    SetTimerSpeedBPM(arm7_globalBpm);
}

//---------------------------------------------------------------------------------

void XMXPlayer_arm7_StopPlaying()
{
    return XM7_StopModule(XM7_Module);
}

//---------------------------------------------------------------------------------

void XMXPlayer_arm7_ModuleManagerHandler(void* pModule, void *userdata)
{
    if (pModule != NULL)
    {
        // received a pointer to a module that should either start or stop now
        XM7_ModuleManager_Type *module = (XM7_ModuleManager_Type*) pModule;
        XM7_Module = module;
        // Start or stop
        (module->State == XM7_STATE_PLAYING) ? XMXPlayer_arm7_StopPlaying() : XMXPlayer_arm7_StartPlaying();
    }
}
