#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"
#include "libxm7.h"
#include "malloc.h"

void arm7_XMXServiceHandler(XMXServiceMsg_S* pService, void *userdata)
{
    /* On SetParams */
    if (pService->Command == CMD_ARM7_SET_PARAMS)
    {
        setGlobalBpm(pService->Bpm);
        setHotCuePos(pService->CuePosition);
        XM7_Module->CurrentTick += pService->Nudge;
    }
}
