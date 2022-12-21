#include <nds.h>
#include <stdio.h>

#include "arm7_defines.h"

#define CMD_SET_BPM_TEMPO   0

extern u8 globalBpm;
extern u8 globalTempo;

void setGlobalBpm(u8 value);
void setGlobalTempo(u8 value);
