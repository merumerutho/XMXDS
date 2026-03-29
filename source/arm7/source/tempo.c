#include <nds.h>

#include "libxm7.h"
#include "tempo.h"
#include "arm7_fifo.h"

u8  arm7_globalBpm            = 125;
u8  arm7_globalHotCuePosition = 0;
u8  arm7_bpmLock              = 0;

/* Loop roll state */
u8  arm7_rollActive        = 0;
u8  arm7_rollN             = 0;
u8  arm7_rollEntry_SongPos = 0;
u8  arm7_rollEntry_PatNum  = 0;
u16 arm7_rollEntry_Line    = 0;
u32 arm7_rollLinesElapsed  = 0;

void setGlobalBpm(u8 value)
{
    arm7_globalBpm = value;
    if (XM7_Module != NULL)
    {
        XM7_Module->CurrentBPM = value;
        SetTimerSpeedBPM(value);
    }
}

void setHotCuePos(u8 value)
{
    arm7_globalHotCuePosition = value;
}

void setBpmLock(u8 enable)
{
    arm7_bpmLock = enable;
}
