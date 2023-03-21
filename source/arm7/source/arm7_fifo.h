#ifndef ARM7_SOURCE_ARM7_FIFO_H_
#define ARM7_SOURCE_ARM7_FIFO_H_

#include <nds.h>

#define CMD_SET_GLOBAL_SETTINGS   0
#define FIFO_XMX                    (FIFO_USER_08)

// Inter-processor communication packet (address sent via FIFO queue)
typedef struct
{
    u8 command;
    u32 data[16];
} __attribute__ ((packed)) XMXServiceMsg;

void arm7_XMXServiceHandler(u32 p, void *userdata);

#endif /* ARM7_SOURCE_ARM7_FIFO_H_ */
