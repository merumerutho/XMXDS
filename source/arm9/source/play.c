#include "play.h"

#include <stdio.h>

#include "arm9_defines.h"
#include "../../arm7/source/arm7_defines.h"

void play_stop()
{
    // Notifying arm7 to begin playing module loaded onto modManager
    if (!fifoSendAddress(FIFO_XM7, deckInfo.modManager))
    {
        iprintf("Could not send data correctly...\n");
    }
}

