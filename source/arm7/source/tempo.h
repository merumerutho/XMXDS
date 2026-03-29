#include <nds.h>
#include <stdio.h>

#include "arm7_defines.h"

extern u8  arm7_globalBpm;
extern u8  arm7_globalHotCuePosition;
extern u8  arm7_bpmLock;

/* Loop roll state */
extern u8  arm7_rollActive;
extern u8  arm7_rollN;
extern u8  arm7_rollEntry_SongPos;
extern u8  arm7_rollEntry_PatNum;
extern u16 arm7_rollEntry_Line;
extern u32 arm7_rollLinesElapsed;

void setGlobalBpm(u8 value);
void setHotCuePos(u8 value);
void setBpmLock(u8 enable);
