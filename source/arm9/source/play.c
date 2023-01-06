#include "play.h"

#include <stdio.h>

#include "arm9_defines.h"
#include "../../arm7/source/arm7_defines.h"

void play_stop(XMX_ModuleInfo *mInfo)
{
    // sending pointer to the libxm7 engine on ARM7
    if (!fifoSendValue32(FIFO_XM7, (u32) mInfo->modManager)) iprintf("Could not send data correctly...\n");
}

