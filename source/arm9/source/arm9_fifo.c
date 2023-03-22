/*
 * arm9_fifo.c
 *
 *  Created on: 21 gen 2023
 *      Author: merut
 */
#include "arm9_fifo.h"
#include "arm9_defines.h"
#include "screens.h"

// Initialize fifo_msg
XMXServiceMsg* ServiceMsg9to7 = NULL;

vu8 arm9_globalBpm = DEFAULT_BPM;
vu8 arm9_globalTempo = DEFAULT_TEMPO;
vu8 arm9_globalHotCuePosition = DEFAULT_CUEPOS;

void arm9_serviceMsgInit()
{
    ServiceMsg9to7 = malloc(sizeof(XMXServiceMsg));
}

void serviceSend(u8 fifo)
{
    fifoSendValue32(fifo, (u32) ServiceMsg9to7);
}

void serviceUpdate(int8 nudge)
{
    ServiceMsg9to7->data[DATA_IDX_BPM] = arm9_globalBpm;
    ServiceMsg9to7->data[DATA_IDX_TEMPO] = arm9_globalTempo;
    ServiceMsg9to7->data[DATA_IDX_HOTCUE] = arm9_globalHotCuePosition;
    ServiceMsg9to7->data[DATA_IDX_NUDGE] = nudge;
    ServiceMsg9to7->command = CMD_ARM7_SET_PARAMS;
    serviceSend(FIFO_XMX);
}

void arm9_XMXServiceHandler(u32 p, void *userdata)
{

}
