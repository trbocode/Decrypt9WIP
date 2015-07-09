#include <3ds.h>
#include <stdlib.h>
#include <stdio.h>
#include "payload_bin.h"
#include "brahma.h"

s32 main (void) {
	srvInit();
	aptInit();
	hidInit(NULL);
	gfxInitDefault();
	fsInit();
	sdmcInit();
	hbInit();
	qtmInit();
	
	if (brahma_init()) {
		load_arm9_payload_from_mem(payload_bin, payload_bin_size);
		set_screen_mode(GSP_BGR8_OES, GSP_BGR8_OES);
		firm_reboot();
		consoleInit(GFX_BOTTOM, NULL);
		brahma_exit();
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
