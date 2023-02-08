#include <nds.h>
#include <stdio.h>

#include "arm7_defines.h"

extern u8 arm7_globalBpm;
extern u8 arm7_globalTempo;
extern u8 arm7_globalHotCuePosition;

void setGlobalBpm(u8 value);
void setGlobalTempo(u8 value);
void setHotCuePos(u8 value);
