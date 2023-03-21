#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"
#include "libxm7.h"
#include "malloc.h"

void arm7_XMXServiceHandler(u32 p, void *userdata)
{
    // Extract data from IPC packet
    u8 command = ((XMXServiceMsg*) (p))->command;

    if (command == CMD_SET_GLOBAL_SETTINGS)
    {
        setGlobalBpm(((XMXServiceMsg*) (p))->data[0]);
        setGlobalTempo(((XMXServiceMsg*) (p))->data[1]);
        setHotCuePos(((XMXServiceMsg*) (p))->data[2]);
        XM7_Module->CurrentTick += ((XMXServiceMsg*) (p))->data[3];
    }

    fifoSendValue32(FIFO_XMX, 0); // ACK
}
