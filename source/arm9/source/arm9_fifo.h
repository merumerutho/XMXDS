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

extern vu8 arm9_globalBpm;
extern vu8 arm9_globalTempo;
extern vu8 arm9_globalHotCuePosition;

void arm9_serviceMsgInit();
void serviceSend(u8 fifo);
void serviceUpdate(int8 nudge);

void arm9_XMXServiceHandler(u32 p, void *userdata);


#endif /* ARM9_SOURCE_ARM9_FIFO_H_ */
