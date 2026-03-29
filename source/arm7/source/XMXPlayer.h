/*
 * XMXPlayer.h
 *
 */

#ifndef ARM7_SOURCE_XMXPLAYER_H_
#define ARM7_SOURCE_XMXPLAYER_H_

#include <nds.h>

#define ARM7_XMXPLAYER_BEAT_COUNTER_TICKS 4  /* must be a power of 2 */

void XMX_Initialize();
void XMXPlayer_arm7_TimerHandler();
void XMXPlayer_arm7_StartPlaying();
void XMXPlayer_arm7_StopPlaying();
void XMXPlayer_arm7_ModuleManagerHandler(void* pModule, void *userdata);

#endif /* ARM7_SOURCE_XMXPLAYER_H_ */
