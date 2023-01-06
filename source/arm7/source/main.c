#include <nds.h>

#include "arm7_defines.h"
#include "libxm7.h"
#include "arm7_fifo.h"

// "reserve" FIFO Channel "FIFO_USER_07"
#define FIFO_XM7    (FIFO_USER_07)

//---------------------------------------------------------------------------------
void VblankHandler(void)
{
    //Wifi_Update();
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
void XM7_arm7_Value32Handler(u32 p, void *userdata)
{
    if (p != 0)
    {
        // received a pointer to a module that should start now
        XM7_ModuleManager_Type *module = (XM7_ModuleManager_Type*) p;
        XM7_Modules[module->moduleIndex] = module;

        if (module->State == XM7_STATE_PLAYING)
            XM7_StopModule(XM7_Modules[module->moduleIndex]);
        else
            XM7_PlayModule(XM7_Modules[module->moduleIndex]);
    }
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

    // Initialize XM7
    XM7_Initialize();

    // Install the FIFO handler for libXM7 "fifo channel"
    fifoSetValue32Handler(FIFO_XM7, XM7_arm7_Value32Handler, 0);

    // Handler for BPM / Tempo
    fifoSetValue32Handler(FIFO_GLOBAL_SETTINGS, arm7_GlobalSettingsFIFOHandler, 0);

    installSystemFIFO();

    irqSet(IRQ_VCOUNT, VcountHandler);
    irqSet(IRQ_VBLANK, VblankHandler);

    irqEnable(IRQ_VBLANK | IRQ_VCOUNT);

    setPowerButtonCB(powerButtonCB);

    // Keep the ARM7 mostly idle
    while (!exitflag)
    {
        if (0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R)))
        {
            exitflag = true;
        }
        swiWaitForVBlank();
    }
    return 0;
}
