#ifndef ARM7_SOURCE_ARM7_FIFO_H_
#define ARM7_SOURCE_ARM7_FIFO_H_

#include <nds.h>

#define PACKED __attribute__ ((packed))

/* Address-based service message — used only for CMD_SET_PARAMS */
typedef struct
{
    u32 Command;        /* Must be CMD_SET_PARAMS */
    u32 Bpm;            /* BPM value */
    u32 CuePosition;    /* Hot cue position */
    s32 Nudge;          /* Signed nudge: -1, 0, or +1 */
} PACKED XMXServiceMsg_S;

/*
 * Value32-based command encoding.
 * Bits [31:24] = command type (u8), bits [23:0] = parameter (s24 or u24).
 *
 *   XMX_MKCMD(cmd, param)  — build a value32 to send via fifoSendValue32
 *   XMX_CMD_TYPE(val)      — extract command type
 *   XMX_CMD_PARAM_U(val)   — extract parameter as unsigned 24-bit
 *   XMX_CMD_PARAM_S(val)   — extract parameter as sign-extended 32-bit signed
 */
#define XMX_MKCMD(cmd, param)   ((((u32)(cmd)) << 24) | ((u32)(s32)(param) & 0x00FFFFFF))
#define XMX_CMD_TYPE(val)       (((val) >> 24) & 0xFF)
#define XMX_CMD_PARAM_U(val)    ((val) & 0x00FFFFFF)
#define XMX_CMD_PARAM_S(val)    ((s32)(((val) << 8) >> 8))

/* Address-based commands */
#define CMD_SET_PARAMS          0

/* Value32-based commands — ARM9 → ARM7 */
#define CMD_SET_TRANSPOSE       1   /* param: s24 absolute transpose value */
#define CMD_SET_LOOPMODE        2   /* param: 0 = off, 1 = on */
#define CMD_GOTO_HOTCUE         3   /* param: u8 target song position */
#define CMD_SET_CHANNEL_MUTE    4   /* param: (channel << 8) | mute_state */
#define CMD_SET_BPM_LOCK        5   /* param: 0 = off, 1 = on */

/* Value32-based commands — ARM7 → ARM9 */
#define CMD_BEAT_PULSE          6   /* param: current line number (informational) */

/* Value32-based commands — ARM9 → ARM7 (roll) */
#define CMD_ROLL_START          7   /* param: N lines (1, 2, 4, 8, or 16) */
#define CMD_ROLL_STOP           8   /* param: unused (0) */

#define FIFO_XMX                (FIFO_USER_08)

void arm7_XMXServiceHandler(void* pMsg, void *userdata);
void arm7_XMXValueHandler(u32 value, void *userdata);

/* Send a beat pulse to ARM9 (called from timer handler) */
void arm7_sendBeatPulse(u8 line);

#endif /* ARM7_SOURCE_ARM7_FIFO_H_ */
