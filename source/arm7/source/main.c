#include <nds.h>

#include "XMXPlayer.h"
#include "arm7_fifo.h"

// "reserve" FIFO Channel "FIFO_USER_07"
#define FIFO_XM7    (FIFO_USER_07)

//---------------------------------------------------------------------------------
void VblankHandler(void)
{
    // Do nothing
}

//---------------------------------------------------------------------------------
void VcountHandler()
{
    inputGetAndSend();
}

volatile bool exitflag = false;

//---------------------------------------------------------------------------------
void powerButtonCB()
{
    exitflag = true;
}

//---------------------------------------------------------------------------------

int main()
{
    // clear sound registers
    dmaFillWords(0, (void*) 0x04000400, 0x100);

    irqInit();
    fifoInit();

    SetYtrigger(80);

    installSoundFIFO();

    // Initialize XMX
    XMX_Initialize();

    // Install the handler function to the FIFO_XM7 queue
    fifoSetValue32Handler(FIFO_XM7, XMXPlayer_arm7_pointerToXmHandler, 0);

    installSystemFIFO();

    irqSet(IRQ_VCOUNT, VcountHandler);
    irqSet(IRQ_VBLANK, VblankHandler);

    irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

    setPowerButtonCB(powerButtonCB);

    // Keep the ARM7 mostly idle
    while (!exitflag)
    {
        swiWaitForVBlank();
    }
    return 0;
}
