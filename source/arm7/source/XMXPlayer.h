/*
 * XMXPlayer.h
 *
 */

#ifndef ARM7_SOURCE_XMXPLAYER_H_
#define ARM7_SOURCE_XMXPLAYER_H_

#include <nds.h>

void XMX_Initialize();
void XMXPlayer_arm7_TimerHandler();
void XMXPlayer_arm7_StartPlaying();
void XMXPlayer_arm7_StopPlaying();
void XMXPlayer_arm7_pointerToXmHandler(u32 p, void *userdata);

#endif /* ARM7_SOURCE_XMXPLAYER_H_ */
