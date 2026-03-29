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
    XM7_Initialize();
}

//---------------------------------------------------------------------------------

static void drainFifoXMX()
{
    // Drain address-based messages (CMD_SET_PARAMS)
    while (fifoCheckAddress(FIFO_XMX))
        arm7_XMXServiceHandler(fifoGetAddress(FIFO_XMX), NULL);

    // Drain value32-based messages (CMD_SET_TRANSPOSE, CMD_SET_LOOPMODE, etc.)
    while (fifoCheckValue32(FIFO_XMX))
        arm7_XMXValueHandler(fifoGetValue32(FIFO_XMX), NULL);
}

//---------------------------------------------------------------------------------

void XMXPlayer_arm7_TimerHandler()
{
    static int beatCounter;
    
    beatCounter += (XM7_Module->CurrentTick == 0);
    beatCounter &= (ARM7_XMXPLAYER_BEAT_COUNTER_TICKS - 1);
    
    // Drain any FIFO_XMX messages that arrived between interrupt deliveries
    drainFifoXMX();

    // Call libxm7 Timer1Handler
    XM7_Timer1Handler();

    if (XM7_Module != NULL && XM7_Module->State == XM7_STATE_PLAYING)
    {
        // BPM lock: re-apply user BPM after each tick to override any Fxx effect
        if (arm7_bpmLock)
            setGlobalBpm(arm7_globalBpm);

        // Beat pulse: notify ARM9 on the note of a beat
        if (beatCounter == 0)
            arm7_sendBeatPulse(XM7_Module->CurrentLine);
    }
}

//---------------------------------------------------------------------------------

void XMXPlayer_arm7_StartPlaying()
{
    // Drain any FIFO_XMX messages queued before playback started
    drainFifoXMX();

    // Set current song position to the hot cue
    XM7_Module->CurrentSongPosition = arm7_globalHotCuePosition;

    // Set current song BPM and Tempo to global values
    XM7_Module->CurrentBPM = arm7_globalBpm;
    XM7_Module->CurrentTempo = XM7_Module->DefaultTempo;

    // Start playing (here the timer callback associated with a libxm7 function)
    XM7_PlayModule(XM7_Module);

    // Immediately override the timer callback with a custom function
    irqSet(IRQ_TIMER1, XMXPlayer_arm7_TimerHandler);

    // Re-apply user BPM (XM7_PlayModule resets it internally)
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
        XM7_ModuleManager_Type *module = (XM7_ModuleManager_Type*) pModule;
        XM7_Module = module;
        (module->State == XM7_STATE_PLAYING) ? XMXPlayer_arm7_StopPlaying() : XMXPlayer_arm7_StartPlaying();
    }
}
