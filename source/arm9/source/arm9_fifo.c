/*
 * arm9_fifo.c
 *
 *  Created on: 21 gen 2023
 *      Author: merut
 */
#include "arm9_fifo.h"
#include "arm9_defines.h"
#include "screens.h"

XMXServiceMsg_S ServiceMsg9to7;

/* Initialize with default values */
vu8 arm9_globalBpm = DEFAULT_BPM;
vu8 arm9_globalTempo = DEFAULT_TEMPO;
vu8 arm9_globalHotCuePosition = DEFAULT_CUEPOS;

/* Ping ARMv7 core with address containing service message to update playback*/
void serviceSend(u8 fifo)
{
    fifoSendAddress(fifo, &ServiceMsg9to7);
}

/* Update values and ping ARMv7 core to update playback */
void serviceUpdate(int8 nudge)
{
    ServiceMsg9to7.Command = CMD_ARM7_SET_PARAMS;
    ServiceMsg9to7.Bpm = arm9_globalBpm;
    ServiceMsg9to7.Tempo = arm9_globalTempo;
    ServiceMsg9to7.CuePosition = arm9_globalHotCuePosition;
    ServiceMsg9to7.Nudge = nudge;
    serviceSend(FIFO_XMX);
}

void arm9_XMXServiceHandler(void * p, void *userdata)
{

}
