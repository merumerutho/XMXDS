#ifndef ARM7_SOURCE_ARM7_FIFO_H_
#define ARM7_SOURCE_ARM7_FIFO_H_

#include <nds.h>

#define ARR_SIZEOF(x) (sizeof(x) / sizeof(u32))
#define PACKED __attribute__ ((packed))

/* ServiceMsg packet*/
typedef struct
{
    u32 Command;        /* Command to execute (e.g. SetParams) */
    u32 Bpm;            /* BPM value */
    u32 Tempo;          /* Tempo value */
    u32 CuePosition;    /* Cue Position value */
    u32 Nudge;          /* Nudge tempo value */
} PACKED XMXServiceMsg_S;

/* ServiceMsg union to have packet as struct, header-payload struct or array of words*/
typedef union
{
    XMXServiceMsg_S asStruct;
    u32 asArray[ARR_SIZEOF(XMXServiceMsg_S)];
    struct
    {
        u32 Command;
        u32 Payload[ARR_SIZEOF(XMXServiceMsg_S)-1];
    } PACKED asPayloadStruct;
} XMXServiceMsg_U;

#define CMD_ARM7_SET_PARAMS             0
#define CMD_ARM9_UPDATE_BPM_TEMPO       64

#define FIFO_XMX                        (FIFO_USER_08)

void arm7_serviceMsgInit();

void arm7_XMXServiceHandler(XMXServiceMsg_S* pMsg, void *userdata);

#endif /* ARM7_SOURCE_ARM7_FIFO_H_ */
