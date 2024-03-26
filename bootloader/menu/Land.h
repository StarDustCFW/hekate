#include "tools/fs_utils.h"
#include "tools/touch2.h"
#include "gui/gui_argon_menu.h"
#include "gui/gui_menu_pool.h"
#include "tools/update.h"
#include "tools/tools.h"
#include "input/touch.h"

int llaunch_payload(char *path)
{
    _launch_payload(path, false, true);
	return 1;
}

void takeoff(){
    display_backlight_brightness(h_cfg.backlight, 1000);

	/* Mount Sd card */
	if (sd_mount())
	{
        //minerva_change_freq(FREQ_1600);		
		Update_SDT();
		clean_up();
        
        anothermain();
        minerva_change_freq(FREQ_800);

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
void anothermain(){

    if (btn_read_vol() == (BTN_VOL_UP)){
        _launch_payload("/StarDust.bin", false, false);
        _launch_payload("/StarDust_update/StarDust.bin", false, false);
    }

    if (h_cfg.rcm_patched) {
        f_unlink("/StarDust/sys/minerva.bso");
        f_unlink("/atmosphere/contents/010000000000000D/exefs.nsp");
        f_unlink("/atmosphere/contents/0100000000001013/exefs.nsp");
        //Start Updated Stardust Menu TODO...240326 ready
        //_launch_payload("StarDust/payloads/hekate.bin", false, false);
        return;
    }

return;
}