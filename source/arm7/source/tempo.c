#include <nds.h>

#include "libxm7.h"
#include "tempo.h"
#include "arm7_fifo.h"

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
