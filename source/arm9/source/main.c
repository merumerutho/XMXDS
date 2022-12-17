
#include <nds.h>
#include <stdio.h>

//---------------------------------------------------------------------------------
int main(void) {
//---------------------------------------------------------------------------------
	// touchPosition touch;

	consoleDemoInit();  // setup the sub screen for printing

	iprintf("XMXDS\n");
	iprintf("merumerutho - sverx\n\n");
	iprintf("github.com/merumerutho/XMXDS\n");
    iprintf("github.com/sverx/libxm7/\n");

    while(1) 
    {
        
    };
    
    /*
	while(1) {

		touchRead(&touch);
		iprintf("\x1b[10;0HTouch x = %04i, %04i\n", touch.rawx, touch.px);
		iprintf("Touch y = %04i, %04i\n", touch.rawy, touch.py);

		swiWaitForVBlank();
		scanKeys();
		if (keysDown()&KEY_START) break;
	}
    */
	return 0;
}
