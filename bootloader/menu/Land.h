#include "tools/fs_utils.h"
#include "tools/touch2.h"
#include "gui/gui_argon_menu.h"
#include "tools/update.h"
#include "tools/tools.h"
#include "input/touch.h"

int llaunch_payload(char *path)
{
    _launch_payload(path, false, true);
	return 1;
}

void takeoff(){
    //gfx_clear_buffer();
    display_backlight_brightness(h_cfg.backlight, 1000);
	/* Mount Sd card */
	if (sd_mount())
	{
        minerva_change_freq(FREQ_1600);//moverall("/amame", "/atmosphere", "*", "test");        cyper("payload.bin","payload.enc");cyper("payload.enc","payload_out.bin");msleep(5000);power_set_state(POWER_OFF);

		Update_SDT();
		clean_up();
        
		/* Cofigure touch input */
		touch_power_on();
        
		gui_init_argon_boot();
		//gui_init_argon_menu();
        
	}
	else
	{
        gfx_con.fntsz = 16;
		gfx_printf( "No sd card found...\n");
        gfx_printf("Something go Wrong\n\n");
        msleep(5000); // Guard against injection VOL+.
        BootStrapNX();
	}
}
