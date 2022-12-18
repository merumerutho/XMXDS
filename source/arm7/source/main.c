#include <nds.h>

#include "libxm7.h"

// "reserve" FIFO Channel "FIFO_USER_07"
#define FIFO_XM7    (FIFO_USER_07)

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	//Wifi_Update();
}


//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB() {
//---------------------------------------------------------------------------------
	exitflag = true;
}

//---------------------------------------------------------------------------------
void XM7_arm7_Value32Handler (u32 command, void* userdata)
{
    if (command!=0)
    {
        // received a pointer to a module that should start now
        XM7_Modules[0] = (XM7_ModuleManager_Type*) command;
        XM7_PlayModule(XM7_Modules[0]);
    }
    else
        // received a 0: stop the module
        XM7_StopModule(XM7_Modules[0]);
}

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	// clear sound registers
	dmaFillWords(0, (void*)0x04000400, 0x100);

    irqInit();
    fifoInit();

    // read User Settings from firmware
    readUserSettings();

    // Start the RTC tracking IRQ
    initClockIRQ();

    SetYtrigger(80);

    //installWifiFIFO();
    installSoundFIFO();

    // Initialize XM7
    XM7_Initialize();

    // Install the FIFO handler for libXM7 "fifo channel"
    fifoSetValue32Handler(FIFO_XM7, XM7_arm7_Value32Handler, 0);

    installSystemFIFO();

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

	setPowerButtonCB(powerButtonCB);

	// Keep the ARM7 mostly idle
	while (!exitflag) {
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			exitflag = true;
		}
		swiWaitForVBlank();
	}
	return 0;
}
