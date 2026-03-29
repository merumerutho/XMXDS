/*
 * arm9_fifo.h
 */

#ifndef ARM9_SOURCE_ARM9_FIFO_H_
#define ARM9_SOURCE_ARM9_FIFO_H_

#include "../../arm7/source/arm7_fifo.h"
#include "../../arm7/source/arm7_defines.h"
#include "arm9_defines.h"

/*
 * ARM9-side shadow state.
 * These are the authoritative values for fields that ARM9 originates.
 * ARM9 keeps these in sync locally and sends them to ARM7 via IPC.
 * ARM9 uses these for display rather than reading back from the module struct.
 */
extern vu8  arm9_globalBpm;
extern vu8  arm9_globalTempo;
extern vs8  arm9_globalTranspose;
extern vu8  arm9_globalLoopMode;
extern vu8  arm9_bpmLock;
extern u8   arm9_channelMute[16];
extern vu8  arm9_beatCounter;   /* decremented each frame; non-zero = beat flash active */
extern vu8  arm9_rollActive;    /* 1 = roll in progress */
extern vu8  arm9_rollN;         /* current roll length in lines */

extern u8   arm9_cuePoints[N_CUES]; /* cue[0] = hot cue (also mapped to Y button) */
extern s8   arm9_soloChannel;       /* -1 = no solo; 0..15 = soloed channel index */
extern u8   arm9_preSoloMute[16];   /* mute snapshot saved before entering solo */

/* Send a full parameter update to ARM7 (BPM, CuePosition, Nudge) */
void serviceUpdate(int8 nudge);

/* Send a value32 command to ARM7 (Transpose, LoopMode, GotoHotCue, ChannelMute) */
void serviceCmd(u32 cmd, s32 param);

/* Initialise arm9_channelMute from the module's initial mute array after loading */
void arm9_initChannelMute(const vu8 *muteArray);

/* ARM9-side FIFO_XMX handlers */
void arm9_XMXServiceHandler(void* p, void *userdata);  /* address-based (reserved) */
void arm9_XMXValueHandler(u32 value, void *userdata);  /* value32-based (beat pulse, etc.) */

#endif /* ARM9_SOURCE_ARM9_FIFO_H_ */
