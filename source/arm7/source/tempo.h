#include <nds.h>
#include <stdio.h>

#include "arm7_defines.h"

extern u8 arm7_globalBpm;
extern u8 arm7_globalTempo;

void setGlobalBpm(u8 value);
void setGlobalTempo(u8 value);
