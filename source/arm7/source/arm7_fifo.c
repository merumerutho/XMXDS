#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"

void arm7_GlobalSettingsFIFOHandler(void* p, void *userdata)
{
    FifoMsg* msg = (FifoMsg*) p;
    u8 command = msg->command;

    // Evaluate
    if (command == CMD_SET_BPM_TEMPO)
    {
        setGlobalBpm(msg->data[0]);
        setGlobalTempo(msg->data[1]);
    }
}
