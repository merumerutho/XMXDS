/*
 * arm9_fifo.h
 *
 *  Created on: 21 gen 2023
 *      Author: merut
 */

#ifndef ARM9_SOURCE_ARM9_FIFO_H_
#define ARM9_SOURCE_ARM9_FIFO_H_

#include "../../arm7/source/arm7_fifo.h"
#include "../../arm7/source/arm7_defines.h"

extern FifoMsg* fifoGlobalMsg;
extern u8 arm9_globalBpm;
extern u8 arm9_globalTempo;
extern u8 arm9_globalCuePosition;

void IpcInit();
void IpcSend(u8 fifo);
void sendBpmTempo(u8 bpm, u8 tempo);


#endif /* ARM9_SOURCE_ARM9_FIFO_H_ */
