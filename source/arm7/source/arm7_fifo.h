#ifndef ARM7_SOURCE_ARM7_FIFO_H_
#define ARM7_SOURCE_ARM7_FIFO_H_

#include <nds.h>

// Inter-processor communication packet (address sent via FIFO queue)
typedef struct
{
    u8 command;
    char data[16];
} __attribute__ ((packed)) IPC_FIFO_packet;

void arm7_GlobalSettingsFIFOHandler(u32 command, void* userdata);

#endif /* ARM7_SOURCE_ARM7_FIFO_H_ */
