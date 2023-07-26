#include <nds.h>

#include "libxm7.h"
#include "tempo.h"
#include "arm7_fifo.h"

// Default starting values for global values

u8 arm7_globalBpm = 125;
u8 arm7_globalHotCuePosition = 0;

void setGlobalBpm(u8 value)
{
    arm7_globalBpm = value;
    // Update value to modules
    XM7_Module->CurrentBPM = value;
    SetTimerSpeedBPM(value);
}

void setHotCuePos(u8 value)
{
    arm7_globalHotCuePosition = value;
}
