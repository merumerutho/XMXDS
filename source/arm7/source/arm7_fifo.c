#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"
#include "libxm7.h"
#include "malloc.h"

void arm7_GlobalSettingsFIFOHandler(u32 p, void *userdata)
{
    // Extract data from IPC packet
    u8 command = ((FifoMsg*) (p))->command;

    if (command == CMD_APPLY_GLOBAL_SETTINGS)
    {
        setGlobalBpm(((FifoMsg*) (p))->data[0]);
        setGlobalTempo(((FifoMsg*) (p))->data[1]);
        setHotCuePos(((FifoMsg*) (p))->data[2]);
    }

    fifoSendValue32(FIFO_XMX, 0); // ACK
}
