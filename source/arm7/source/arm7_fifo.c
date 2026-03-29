#include "arm7_fifo.h"

#include "arm7_defines.h"
#include "tempo.h"
#include "libxm7.h"

void arm7_XMXServiceHandler(void* pMsg, void *userdata)
{
    XMXServiceMsg_S* pService = (XMXServiceMsg_S*) pMsg;
    if (pService->Command == CMD_SET_PARAMS)
    {
        setGlobalBpm(pService->Bpm);
        setHotCuePos(pService->CuePosition);
        if (XM7_Module != NULL)
            XM7_Module->CurrentTick += pService->Nudge;
    }
}

void arm7_XMXValueHandler(u32 value, void *userdata)
{
    u8  cmd   = XMX_CMD_TYPE(value);
    u32 param = XMX_CMD_PARAM_U(value);

    if (XM7_Module == NULL)
        return;

    switch (cmd)
    {
        case CMD_SET_TRANSPOSE:
            XM7_Module->Transpose = (s8) XMX_CMD_PARAM_S(value);
            break;

        case CMD_SET_LOOPMODE:
            XM7_Module->LoopMode = (u8) param;
            break;

        case CMD_GOTO_HOTCUE:
            XM7_Module->CurrentSongPosition = (u8) param;
            XM7_Module->bGotoHotCue = TRUE;
            break;

        case CMD_SET_CHANNEL_MUTE:
        {
            u8 channel = (u8)((param >> 8) & 0x0F);
            u8 mute    = (u8)(param & 0x01);
            XM7_Module->ChannelMute[channel] = mute;
            break;
        }

        case CMD_SET_BPM_LOCK:
            setBpmLock((u8) param);
            break;
    }
}

void arm7_sendBeatPulse(u8 line)
{
    fifoSendValue32(FIFO_XMX, XMX_MKCMD(CMD_BEAT_PULSE, line));
}
