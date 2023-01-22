#ifndef ARM7_SOURCE_ARM7_FIFO_H_
#define ARM7_SOURCE_ARM7_FIFO_H_

#include <nds.h>

#define CMD_SET_BPM_TEMPO   0
#define CMD_MUTE_CHANNEL    1
#define CMD_DUMMY           2

// Inter-processor communication packet (address sent via FIFO queue)
typedef struct
{
    u8 command;
    u32 data[16];
} __attribute__ ((packed)) FifoMsg;

void arm7_GlobalSettingsFIFOHandler(u32 p, void *userdata);

#endif /* ARM7_SOURCE_ARM7_FIFO_H_ */
