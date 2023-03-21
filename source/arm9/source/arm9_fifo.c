/*
 * arm9_fifo.c
 *
 *  Created on: 21 gen 2023
 *      Author: merut
 */
#include "arm9_fifo.h"
#include "arm9_defines.h"

// Initialize fifo_msg
XMXServiceMsg *fifoGlobalMsg = NULL;

vu8 arm9_globalBpm = DEFAULT_BPM;
vu8 arm9_globalTempo = DEFAULT_TEMPO;
vu8 arm9_globalHotCuePosition = DEFAULT_CUEPOS;

void IpcInit()
{
    fifoGlobalMsg = malloc(sizeof(XMXServiceMsg));
}

void IpcSend(u8 fifo)
{
    fifoSendValue32(fifo, (u32) fifoGlobalMsg);
}

void sendBpmTempo(u8 bpm, u8 tempo)
{
    fifoGlobalMsg->data[0] = bpm;
    fifoGlobalMsg->data[1] = tempo;
    fifoGlobalMsg->command = CMD_SET_GLOBAL_SETTINGS;
    IpcSend(FIFO_XMX);
}
