/*
 * arm9_fifo.c
 *
 *  Created on: 21 gen 2023
 *      Author: merut
 */
#include "arm9_fifo.h"
#include "arm9_defines.h"

// Initialize fifo_msg
FifoMsg* fifoGlobalMsg = NULL;

u8 arm9_globalBpm = DEFAULT_BPM;
u8 arm9_globalTempo = DEFAULT_TEMPO;

void IpcInit()
{
    fifoGlobalMsg = malloc(sizeof(FifoMsg));
}

void IpcSend(u8 fifo)
{
    fifoSendValue32(fifo, (u32) fifoGlobalMsg);
}

void sendBpmTempo(u8 bpm, u8 tempo)
{
    fifoGlobalMsg->data[0] = bpm;
    fifoGlobalMsg->data[1] = tempo;
    fifoGlobalMsg->command = CMD_SET_BPM_TEMPO;
    IpcSend(FIFO_GLOBAL_SETTINGS);
}
