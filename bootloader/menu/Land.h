#include "tools/fs_utils.h"
#include "tools/touch.h"
#include "gui/gui_argon_menu.h"
#include "gui/gui_menu_pool.h"
#include "tools/update.h"
#include "tools/tools.h"

void takeoff(){
	/* Mount Sd card */
	if (sd_mount())
	{
		/* Cofigure touch input */
		touch_power_on();
		
		Update_SDT();
		clean_up();
			
		gui_init_argon_boot();
		//gui_init_argon_menu();
	}
	else
	{
		gfx_printf( "No sd card found...\n");
	}
}
