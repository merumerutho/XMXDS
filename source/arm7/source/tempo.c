#include <nds.h>

#include "libxm7.h"
#include "tempo.h"
#include "arm7_fifo.h"

// Default starting values for BPM and tempo.

u8 arm7_globalBpm = 125;
u8 arm7_globalTempo = 6;

void setGlobalBpm(u8 value)
{
    arm7_globalBpm = value;
    // Update value to modules
    SetTimerSpeedBPM(arm7_globalBpm);
}

void setGlobalTempo(u8 value)
{
    arm7_globalTempo = value;
    // Update value to modules
    for (u8 mm = 0; mm < LIBXM7_ALLOWED_MODULES; mm++)
    {
        XM7_Module->CurrentTempo = arm7_globalTempo;
    }
}
