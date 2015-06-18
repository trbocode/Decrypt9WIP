#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "payload_bin.h"
#include "brahma.h"
#include "bot_gfx_bin.h"


s32 main (void) {
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInitDefault();
	fsInit();
	sdmcInit();
	hbInit();
	qtmInit();

	//We don't need double buffering. In this way we can draw our image only once on screen.
	gfxSetDoubleBuffering(GFX_BOTTOM, false);

	//Get the bottom screen's frame buffer
	u8* fb = gfxGetFramebuffer(GFX_BOTTOM, GFX_LEFT, NULL, NULL);
	
	//Copy our image in the bottom screen's frame buffer
	memcpy(fb, bot_gfx_bin, bot_gfx_bin_size);
	
	if (brahma_init()) {
		load_arm9_payload_from_mem(payload_bin, payload_bin_size);
		firm_reboot();
		brahma_exit();
	}
	
	while (aptMainLoop())
	{
		// Flush and swap framebuffers
		gfxFlushBuffers();
		gfxSwapBuffers();

		//Wait for VBlank
		gspWaitForVBlank();
	}
	
	hbExit();
	sdmcExit();
	fsExit();
	gfxExit();
	hidExit();
	aptExit();
	srvExit();
	
	return 0;
}
