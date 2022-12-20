#include <nds.h>

#include "libxm7.h"
#include "tempo.h"

// Default starting values for BPM and tempo are 0. They must be sent by ARMv9
u8 globalBpm = 0;
u8 globalTempo = 0;

void setGlobalBpm(u8 value)
{
    globalBpm = value;
    // Update value to modules
    SetTimerSpeedBPM (globalBpm);
}

void setGlobalTempo(u8 value)
{
    globalTempo = value;
    // Update value to modules
    for (u8 mm=0; mm < LIBXM7_ALLOWED_MODULES; mm++)
    {
        XM7_Modules[mm]->CurrentTempo = globalTempo;
    }
}

void arm7_TempoFIFOHandler(u32 command, void* userdata)
{
    u8 value = (command) & 0xFF;
    u8 target = (command >> 8) & 0xFF;
    if (target == FIFO_TARGET_BPM)
    {
        setGlobalBpm(value);
    }

    if(target == FIFO_TARGET_TEMPO)
    {
        setGlobalTempo(value);
    }
}
