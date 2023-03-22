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
    // Extract data from IPC packet
    u8 command = ((XMXServiceMsg*) (p))->command;
    u32* data = ((XMXServiceMsg*) (p))->data;

    if (command == CMD_ARM7_SET_PARAMS)
    {
        setGlobalBpm(data[0]);
        setHotCuePos(data[2]);
        XM7_Module->CurrentTick += data[3];
    }
}
