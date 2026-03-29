/*
 * arm9_fifo.c
 */
#include "arm9_fifo.h"
#include "arm9_defines.h"

/* Address-based service message buffer (CMD_SET_PARAMS only) */
static XMXServiceMsg_S ServiceMsg9to7;

/* ARM9 shadow state — values that ARM9 originates and sends to ARM7 via IPC */
vu8 arm9_globalBpm            = DEFAULT_BPM;
vu8 arm9_globalTempo          = DEFAULT_TEMPO;
vu8 arm9_globalHotCuePosition = DEFAULT_CUEPOS;
vs8 arm9_globalTranspose      = 0;
vu8 arm9_globalLoopMode       = 0;
vu8 arm9_bpmLock              = 0;
u8  arm9_channelMute[16]      = {0};
vu8 arm9_beatCounter          = 0;

/* Send full parameter update (BPM, CuePosition, Nudge) to ARM7 via address message */
void serviceUpdate(int8 nudge)
{
    ServiceMsg9to7.Command     = CMD_SET_PARAMS;
    ServiceMsg9to7.Bpm         = arm9_globalBpm;
    ServiceMsg9to7.CuePosition = arm9_globalHotCuePosition;
    ServiceMsg9to7.Nudge       = nudge;

    // Flush ARM9 cache so ARM7 reads up-to-date data from main RAM
    DC_FlushRange(&ServiceMsg9to7, sizeof(ServiceMsg9to7));
    fifoSendAddress(FIFO_XMX, &ServiceMsg9to7);
}

/* Send a value32 command to ARM7 — no shared memory, fully self-contained */
void serviceCmd(u32 cmd, s32 param)
{
    fifoSendValue32(FIFO_XMX, XMX_MKCMD(cmd, param));
}

/* Copy initial mute state from the freshly loaded module into the ARM9 shadow */
void arm9_initChannelMute(const vu8 *muteArray)
{
    for (u8 i = 0; i < 16; i++)
        arm9_channelMute[i] = muteArray[i];
}

void arm9_XMXServiceHandler(void* p, void *userdata)
{
    /* Reserved for ARM7→ARM9 address-based notifications */
}

/* Handles value32 messages sent from ARM7 to ARM9 */
void arm9_XMXValueHandler(u32 value, void *userdata)
{
    switch (XMX_CMD_TYPE(value))
    {
        case CMD_BEAT_PULSE:
            arm9_beatCounter = 3;   /* flash for 3 display frames (~50 ms at 60 Hz) */
            break;
    }
}
