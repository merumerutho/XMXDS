#include <nds.h>
#include <stdlib.h>

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
    // clear sound registers
    dmaFillWords(0, (void*)0x04000400, 0x100);

    REG_SOUNDCNT |= SOUND_ENABLE;
    writePowerManagement(PM_CONTROL_REG, ( readPowerManagement(PM_CONTROL_REG) & ~PM_SOUND_MUTE ) | PM_SOUND_AMP );
    powerOn(POWER_SOUND);

    readUserSettings();
    ledBlink(0);

    irqInit();
    // Start the RTC tracking IRQ
    initClockIRQ();
    fifoInit();
    touchInit();

    SetYtrigger(80);
    installSoundFIFO();

    installSystemFIFO();

    //irqSet(IRQ_VCOUNT, VcountHandler);
    //irqSet(IRQ_VBLANK, VblankHandler);

    //irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

    //setPowerButtonCB(powerButtonCB);

    // Keep the ARM7 mostly idle
    /*
    while (!exitflag) {
        if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
            exitflag = true;
        }
        swiWaitForVBlank();
    }
    */
    return 0;
}