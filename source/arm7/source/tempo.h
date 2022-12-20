#include <nds.h>
#include <stdio.h>

#define FIFO_TEMPO (FIFO_USER_08)

#define FIFO_TARGET_BPM   0
#define FIFO_TARGET_TEMPO 1

extern u8 globalBpm;
extern u8 globalTempo;

void setGlobalBpm(u8 value);
void setGlobalTempo(u8 value);

void arm7_TempoFIFOHandler(u32 command, void* userdata);
