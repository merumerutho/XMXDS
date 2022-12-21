#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"

void arm7_GlobalSettingsFIFOHandler(u32 p, void* userdata)
{
    // Extract data from IPC packet
    u8 command = ((IPC_FIFO_packet*) (p))->command;

    // Evaluate
    if (command == CMD_SET_BPM_TEMPO)
    {
        setGlobalBpm(((IPC_FIFO_packet*) (p))->data[0]);
        setGlobalTempo(((IPC_FIFO_packet*) (p))->data[1]);
    }
}
