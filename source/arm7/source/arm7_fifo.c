#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"
#include "libxm7.h"
#include "malloc.h"


void arm7_XMXServiceHandler(XMXServiceMsg* p, void *userdata)
{
    // Extract data from service msg
    u8 command = p->command;
    u32* data = p->data;

    if (command == CMD_ARM7_SET_PARAMS)
    {
        setGlobalBpm(data[0]);
        setHotCuePos(data[2]);
        XM7_Module->CurrentTick += data[3];
    }
}
