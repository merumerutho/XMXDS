#ifndef ARM9_SOURCE_ARM9_DEFINES_H_
#define ARM9_SOURCE_ARM9_DEFINES_H_

#define MAX(a,b)    ((a>b)?a:b)
#define MIN(a,b)    ((a>b)?b:a)

// FIFO 07 for libxm7
#define FIFO_XM7    (FIFO_USER_07)
#define FIFO_XMX    (FIFO_USER_08)

#define SCREEN_WIDTH    256
#define SCREEN_HEIGHT   192

// Default BPM / TEMPO definitions
#define DEFAULT_BPM     125
#define DEFAULT_TEMPO   6
#define DEFAULT_CUEPOS  0
#define DEFAULT_NUDGE   0

// Number of cue points
#define N_CUES  8

// Bottom screen display modes
typedef enum {
    SCREEN_MODE_CH  = 0,  // 4x4 waveform grid with mute/solo controls
    SCREEN_MODE_CUE = 1,  // Cue points grid + tap tempo
} ScreenMode;

#endif /* ARM9_SOURCE_ARM9_DEFINES_H_ */
