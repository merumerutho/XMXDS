#ifndef ARM7_SOURCE_ARM7_FIFO_H_
#define ARM7_SOURCE_ARM7_FIFO_H_

#include <nds.h>

#define CMD_ARM7_SET_PARAMS             0
#define CMD_ARM9_UPDATE_BPM_TEMPO       64

#define DATA_IDX_BPM                    0
#define DATA_IDX_TEMPO                  1
#define DATA_IDX_HOTCUE                 2
#define DATA_IDX_NUDGE                  3

#define FIFO_XMX                        (FIFO_USER_08)

// Service Msg (address sent via FIFO queue)
typedef struct
{
    u8 command;
    u32 data[4];
} XMXServiceMsg;

extern XMXServiceMsg* ServiceMsg7to9;

void arm7_serviceMsgInit();
void arm7_XMXServiceHandler(u32 p, void *userdata);

#endif /* ARM7_SOURCE_ARM7_FIFO_H_ */
