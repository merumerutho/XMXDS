#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"
#include "libxm7.h"
#include "malloc.h"

XMXServiceMsg* ServiceMsg7to9 = NULL;

void arm7_serviceMsgInit()
{
    ServiceMsg7to9 = malloc(sizeof(XMXServiceMsg));
}

void arm7_XMXServiceHandler(u32 p, void *userdata)
{
    u8 command = ((XMXServiceMsg*) (p))->command;
    u32* data = ((XMXServiceMsg*) (p))->data;

    if (command == CMD_ARM7_SET_PARAMS)
    {
        setGlobalBpm(data[DATA_IDX_BPM]);
        setHotCuePos(data[DATA_IDX_HOTCUE]);

        XM7_Module->CurrentTick += data[DATA_IDX_NUDGE];
    }
}
