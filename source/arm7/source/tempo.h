#include <nds.h>
#include <stdio.h>

#include "arm7_defines.h"

extern u8 arm7_globalBpm;
extern u8 arm7_globalHotCuePosition;
extern u8 arm7_bpmLock;

void setGlobalBpm(u8 value);
void setHotCuePos(u8 value);
void setBpmLock(u8 enable);
