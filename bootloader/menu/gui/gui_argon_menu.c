/*  
 * Copyright (c) 2018 Guillem96
 *
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdlib.h>

#include "power/max17050.h"
#include <string.h>
#include "gui_argon_menu.h"
#include "gui_menu.h"
#include "custom-gui.h"
#include "display/di.h"
#include "../../gfx/gfx.h"
#include <stdio.h>
#include "utils/types.h"
#include "../tools/fs_utils.h"
//#include "utils/dirlist.h"
#include "utils/util.h"
#include "../tools/touch2.h"
#include "utils/btn.h"
#include "custom-gui.h"
#include "mem/heap.h"
//#include "minerva/minerva.h"
#include "../tools/tools.h"
#include "../../storage/emummc.h"

#define COLUMNS 4
#define ROWS 2
#define ELEM_SIZE 230
#define MARGIN_TOP 100
#define MARGIN_LEFT 46
#define MAX_CHAR 100 typedef char string[MAX_CHAR + 1];

//extern hekate_config h_cfg;
extern emummc_cfg_t emu_cfg;

extern char Sversion[4];
extern void _stock_launch();
extern void _cfw(bool emummc);

u32 brillo = 100;
u32 brilloM = 50;

const char *sysserial;
const char *emuserial;
char *buffer_blk;
void upd_menus();
//menus
int main_menu = 0;

//low icons Y
u64 low_icons = 645;

//some indicators
u32 Incac = 0;
u32 servstep = 0;
u32 isAMS = 1;
u32 iamsafe=0;
bool syspatch = true;

#define max_menu 4 

gui_menu_t *menus[max_menu];


void upd_menus(){
    menus[0] = gui_menu_create("ArgonNX","background.bmp");
    menus[1] = gui_menu_create("ArgonNX","back-set.bmp");
    menus[2] = gui_menu_create("ArgonNX","back-inc.bmp");
}

/* Init Boot screen */
void gui_init_argon_boot(void)
{
	SDStrap();
	loadTheme();
    
    //Iinit Menus
    upd_menus();
	
	// Syspatch status
	syspatch = sd_file_exists("/atmosphere/contents/420000000000000B/flags/boot2.flag");

	/* Init pool for menu */
	if (sd_file_exists("StarDust/autoboot.inc"))
	{
		main_menu = 2;
		f_unlink("StarDust/autoboot.inc");
		Incac = 1;
	}
	if (sd_file_exists("StarDust/flags/IamSafe.flag"))
	{
		f_unlink("StarDust/flags/IamSafe.flag");
		iamsafe = 1;
	}
	
	change_brightness(0);

	//show display without icons
	if ((Incac != 1) & (iamsafe != 1)) {
        gui_menu_boot(menus[main_menu]);
	}

	//if (res > 0){gfx_swap_buffer();}
	//corrections on start
	f_mkdir("StarDust/flags");
	f_unlink("auto.bak");

	//set archive bit
	f_chmod("switch/LinkUser", AM_ARC, AM_ARC);

	//profile redo
	if (sd_file_exists("/atmosphere/contents/0100000000001013/exefs.on"))
	{
		f_move("/atmosphere/contents/0100000000001013/exefs.on", "/atmosphere/contents/0100000000001013/exefs.nsp");
		f_unlink("/atmosphere/contents/0100000000001013/exefs.on");
	}

	//prevent accidental boot to ofw
	if (sd_file_exists("bootloader/hekate_ipl.bkp"))
	{
		f_unlink("bootloader/hekate_ipl.ini");
		f_rename("bootloader/hekate_ipl.bkp", "bootloader/hekate_ipl.ini");
	}
 
	//waith user input
	if (sd_file_exists("StarDust/autobootecho.txt") & (Incac == 0) & (iamsafe != 1))
		btn_wait_timeout(1000, BTN_VOL_UP);

	bool cancel_auto_chainloading = btn_read() & BTN_VOL_UP;
	if (sd_file_exists("StarDust/autobootecho.txt") && (!cancel_auto_chainloading) & (Incac == 0) & (iamsafe != 1))
	{
		//autoboot
		if (strstr(Sversion, "fus") != NULL || strstr(Sversion, "Atm") != NULL )
			launcher("StarDust/payloads/fusee.bin");

		if (strstr(Sversion, "S") != NULL)
			launcher("StarDust/payloads/SXOS.bin");

		if (sd_file_exists("/switchroot/ubuntu/coreboot.rom") && (strstr(Sversion, "Ub") != NULL))
		{
			launcher("switchroot/ubuntu/coreboot.rom");
		}

		if (sd_file_exists("/switchroot/android/coreboot.rom") && (strstr(Sversion, "TW") != NULL))
		{
			launcher("switchroot/android/coreboot.rom");
		}
	}
	f_unlink("StarDust/payload.bin");

	if (btn_read() & BTN_VOL_DOWN)
		f_unlink("StarDust/autobootecho.txt");

}

void pre_load_menus(int menuses, bool StarUp)
{
	SDStrap();
	if (menuses == 0 || StarUp)
	{
        menus[0]->next_entry = 0;
		//generate main menu
		u32 main_iconY = 200;
		u32 main_iconX = 190;
		u32 main_iconXS = 300;

		//Create emummc icon
		u32 buttonY = main_iconY - 67;
		if (sd_file_exists("emummc/emummc.ini")){
			if(emu_cfg.enabled) {
				// main_iconX = main_iconX + main_iconXS/2;
				create(menus[0], "Icons/bon.bmp", main_iconX + 50, buttonY, (int (*)(void *))tool_emu, (void *)0); //- 80, - 500
			} else {
				create(menus[0], "Icons/boff.bmp", main_iconX + 50, buttonY, (int (*)(void *))tool_emu, (void *)1); //- 80, - 500
			}
		}

		// Atmosphere Boot
		create(menus[0], "Icons/Atmosphere.bmp", main_iconX, main_iconY, (int (*)(void *))launcher, (void *)"/StarDust/payloads/fusee.bin");
		
		main_iconX = main_iconX + main_iconXS;

		//			{
		//			gui_menu_append_entry(menus[0],gui_create_menu_entry("",theme("Icons/ReiNX.bmp"), main_iconX, main_iconY, 300 , 300,(int (*)(void *))launcher, (void*)"/StarDust/payloads/ReiNX.bin"));
		main_iconX = main_iconX + main_iconXS;
		//			}

		// Stock Boot
		create(menus[0], "Icons/Stock.bmp", 540, main_iconY + 100, (int (*)(void *))_stock_launch, (void *)1);
		//create(menus[0], "Icons/SXOS.bmp", main_iconX, main_iconY, (int (*)(void *))launcher, (void *)"/StarDust/payloads/SXOS.bin");

		
		// Android and Ubuntu 
		if (sd_file_exists("/switchroot/android/coreboot.rom"))
			create(menus[0], "Icons/Android.bmp", 590, main_iconY - 30, (int (*)(void *))launcher, (void *)"/switchroot/android/coreboot.rom");
		else if (sd_file_exists("/switchroot_android/coreboot.rom"))
			create(menus[0], "Icons/Android.bmp", 590, main_iconY - 30, (int (*)(void *))launcher, (void *)"/switchroot_android/coreboot.rom");

		if (sd_file_exists("/switchroot/ubuntu/coreboot.rom"))
			create(menus[0], "Icons/Ubuntu.bmp", 590, main_iconY + 230, (int (*)(void *))launcher, (void *)"/switchroot/ubuntu/coreboot.rom");
		
		// Hekate icon
		create(menus[0], "Icons/Hekate.bmp", main_iconX, main_iconY, (int (*)(void *))launcher, (void *)"/StarDust/payloads/hekate.bin");
/*
		u64 iconrowY = low_icons-5;
		u64 iconrowX = 130;
		u64 iconrowXS = 250;
		create(menus[0], "Icons/Lockpick_RCM.bmp", iconrowX, iconrowY, (int (*)(void *))launcher, (void *)"/StarDust/payloads/Lockpick_RCM.bin");
		iconrowX = iconrowX + iconrowXS;
		create(menus[0], "Icons/TegraEX.bmp", iconrowX, iconrowY, (int (*)(void *))launcher, (void *)"/StarDust/payloads/TegraExplorer.bin");
		iconrowX = iconrowX + iconrowXS;
		iconrowX = iconrowX + 80;
		if (Incac == 0)
			create(menus[0], "Icons/Incognito.bmp", iconrowX, iconrowY, (int (*)(void *))Incognito, (void *)"1");
		else
			create(menus[0], "Icons/Incognito.bmp", iconrowX, iconrowY, (int (*)(void *))tool_Menus, (void *)4);
		iconrowX = iconrowX + iconrowXS;
		//	gui_menu_append_entry(menus[0],gui_create_menu_entry("",theme("Icons/Incognito.bmp"),iconrowX+700, iconrowY, 200 , 200,(int (*)(void *))tool_Menus, (void*)6));
*/

		create(menus[0], "Icons/SD.bmp", 10, low_icons, tool_extr_rSD, NULL);
		//create(menus[0], "Icons/rcm.bmp", 700, low_icons, (int (*)(void *))launcher, (void *)"payload.bin");
		if (iamsafe == 0)
			create(menus[0], "Icons/themes.bmp", 605, low_icons, tool_menu_rem, NULL);
		//			gui_menu_append_entry(menus[0],gui_create_menu_entry("",theme("Icons/Reinx-off.bmp"),366, main_iconY, 300 , 300, NULL, NULL));
		//call menu 1
		create(menus[0], "Icons/gear.bmp", 1200, low_icons, (int (*)(void *))tool_Menus, (void *)1);
        static_menu_elements(menus[0]);
        //create_switch(menus[0], "", "Icons/day.bmp", "Icons/nay.bmp", !sd_file_exists("StarDust/flags/b50.flag"), 1200, 100, (int (*)(void *))medislay, (void *)0, (int (*)(void *))medislay, (void *)0);
	}

	if (menuses == 1 || StarUp)
	{
		static int start_point = 0;
		if (menus[1]->next_entry == 0 || StarUp)
		{
            menus[1]->next_entry = 0;
			//list custom skins
			AThemes_list(menus[1], 80, 90);
            
            //incognito icon
            if (Incac == 0)
                create(menus[1], "Icons/Incognito.bmp", 400, 600, (int (*)(void *))Incognito, (void *)"1");
            else
                create(menus[1], "Icons/Incognito.bmp", 400, 600, (int (*)(void *))tool_Menus, (void *)2);
            
            //tegra explorer
            create(menus[1], "Icons/TegraEX.bmp", 620, 600, (int (*)(void *))launcher, (void *)"/StarDust/payloads/TegraExplorer.bin");
            
            //lockpick
            if (sd_file_exists("/StarDust/payloads/Lockpick_RCM.bin"))
                create(menus[1], "Icons/Lockpick_RCM.bmp", 840, 600, (int (*)(void *))launcher, (void *)"/StarDust/payloads/Lockpick_RCM.bin");

            
            //Create SXOS transfer buttons
            /*
                if (sd_file_exists("emuMMC/EF00/eMMC/00") & sd_file_exists("emuMMC/EF00/eMMC/boot0") & sd_file_exists("emuMMC/EF00/eMMC/boot1"))
                    create(menus[1], "Icons/swap.bmp", 380, 600, (int (*)(void *))tool_emu, (void *)66);

                if (sd_file_exists("sxos/emunand/full.00.bin") & sd_file_exists("sxos/emunand/boot0.bin") & sd_file_exists("sxos/emunand/boot1.bin"))
                    create(menus[1], "Icons/Transfer.bmp", 380, 600, (int (*)(void *))tool_emu, (void *)33); //- 80, - 500

                if (!sd_file_exists("emummc/emummc.ini"))
                    create(menus[1], "Icons/link_hide.bmp", 580, 600, (int (*)(void *))tool_emu, (void *)99);
            
            */
            static_menu_elements(menus[1]);
			//draw power
			create(menus[1], "Icons/power.bmp", 80, 500, tool_power_off, NULL);
            create(menus[1], "Icons/home.bmp", 10, low_icons, (int (*)(void *))tool_Menus, (void *)0);

			start_point = menus[1]->next_entry;
		}
		else
		{
			menus[1]->next_entry = start_point + 1;
		}


		//dinamic
		/*
			menus[1]->entries[menu->next_entry] = menu_entry;
			menus[1]->next_entry++;
		*/
		//draw autoboot
		if (sd_file_exists("StarDust/autobootecho.txt"))
			create(menus[1], "Icons/Auton.bmp", 1050, 600, (int (*)(void *))Autoboot, (void *)0);
		else
			create(menus[1], "Icons/Autoff.bmp", 1050, 600, (int (*)(void *))Autoboot, (void *)1);

		/*

		//draw Ulaunch
		if (sd_file_exists("StarDust/flags/ulaunch.flag"))
			create(menus[1], "Icons/ulon.bmp", 850, 600, (int (*)(void *))uLaunch, (void *)0);
		else
			create(menus[1], "Icons/uloff.bmp", 850, 600, (int (*)(void *))uLaunch, (void *)1);

		//draw CFW select
		if (isAMS)
			create(menus[1], "Icons/serv-ams.bmp", 350, 165, (int (*)(void *))serv_CFW, (void *)0);
		else
			create(menus[1], "Icons/serv-sxos.bmp", 350, 165, (int (*)(void *))serv_CFW, (void *)1);
		*/

		//services
		//		gui_menu_append_entry(menus[1],gui_create_menu_entry_no_bitmap("Services",servX+30, servY-10, 150, 100, NULL, NULL));
		servstep = 0;
		serv_display(menus[1], "420000000000000B", "SigPatch");
		serv_display(menus[1], "010000000000bd00", "MissionC");
		serv_display(menus[1], "420000000007E51A", "TeslaOVL");
		serv_display(menus[1], "420000000000000E", "FTP");
		serv_display(menus[1], "690000000000000D", "Sys-Con");
		serv_display(menus[1], "4200000000000010", "LanPlay");
		serv_display(menus[1], "00FF0000A53BB665", "SysDVR");
		serv_display(menus[1], "0000000000534C56", "ReverseNX");
		serv_display(menus[1], "00FF747765616BFF", "sys-tweak");
		serv_display(menus[1], "00FF0000636C6BFF", "Sys-Clk");
		serv_display(menus[1], "430000000000000B", "sys-botbase");
		serv_display(menus[1], "4200000000000000", "SysTune");
		serv_display(menus[1], "0100000000000352", "Emuiio");
		serv_display(menus[1], "0100000000000081", "nx-btred");
		//serv_display(menus[1],"0100000000000BEF","Disk-USB");
		//serv_display(menus[1],"0100000000000069","ReiSpoof");
		//serv_display(menus[1],"0100000000000FAF","HDI");

		//themes for cfw
		u32 temX = 310;
		u32 temY = 440;
		u32 temS = 300;
		if (!sd_file_exists("atmosphere/contents/0100000000001000/fsmitm.flag"))
			create(menus[1], "Icons/sw-off.bmp", temX, temY, (int (*)(void *))tool_Themes_on, (void *)"atmosphere");
		else
			create(menus[1], "Icons/sw-on.bmp", temX, temY, (int (*)(void *))tool_Themes_off, (void *)"atmosphere");
		//gui_menu_append_entry(menus[1], gui_create_menu_entry_no_bitmap("AMS", temX + 30, temY + 30, 150, 100, NULL, NULL));
		
		create_no_bitmap(menus[1],"AMS",temX + 100, temY + 30);

		temX = temX + temS;
        
        //draw Ulaunch
		if (sd_file_exists("StarDust/flags/ulaunch.flag"))
			create(menus[1], "Icons/ulon.bmp", temX, temY, (int (*)(void *))uLaunch, (void *)0);
		else
			create(menus[1], "Icons/uloff.bmp", temX, temY, (int (*)(void *))uLaunch, (void *)1);

/*
		if (!sd_file_exists("sxos/titles/0100000000001000/fsmitm.flag"))
			create(menus[1], "Icons/sw-off.bmp", temX, temY, (int (*)(void *))tool_Themes_on, (void *)"sxos");
		else
			create(menus[1], "Icons/sw-on.bmp", temX, temY, (int (*)(void *))tool_Themes_off, (void *)"sxos");
		gui_menu_append_entry(menus[1], gui_create_menu_entry_no_bitmap("SXOS", temX + 30, temY + 30, 150, 100, NULL, NULL));
		temX = temX + temS;
		if (isAMS)
		{
			if (!sd_file_exists("atmosphere/contents/0100000000001013/exefs.nsp"))
				create(menus[1], "Icons/sw-off.bmp", temX, temY, (int (*)(void *))tool_Themes_on, (void *)"Profile");
			else
				create(menus[1], "Icons/sw-on.bmp", temX, temY, (int (*)(void *))tool_Themes_off, (void *)"Profile");
		}
		else
		{
			if (!sd_file_exists("sxos/titles/0100000000001013/exefs.nsp"))
				create(menus[1], "Icons/sw-off.bmp", temX, temY, (int (*)(void *))tool_Themes_on, (void *)"Profile");
			else
				create(menus[1], "Icons/sw-on.bmp", temX, temY, (int (*)(void *))tool_Themes_off, (void *)"Profile");
		}

		gui_menu_append_entry(menus[1], gui_create_menu_entry_no_bitmap("Profile", temX + 30, temY + 30, 150, 100, NULL, NULL));
*/

		//Ajustar brillo
        
        create_switch(menus[1], "", "Icons/day.bmp", "Icons/nay.bmp", !sd_file_exists("StarDust/flags/b50.flag"), 1200, 100, (int (*)(void *))medislay, (void *)0, (int (*)(void *))medislay, (void *)0);
        /*
		if (!sd_file_exists("StarDust/flags/b50.flag"))
			create(menus[1], "Icons/day.bmp", 1200, 100, (int (*)(void *))medislay, (void *)0);
		else
			create(menus[1], "Icons/nay.bmp", 1200, 100, (int (*)(void *))medislay, (void *)0);
        */
	}

	if (menuses == 2)
	{
        menus[2]->next_entry = 0;

		//		incognito togle
		//Getinfo from text
		u32 sys_inc = 0;
		u32 emu_inc = 0;

		//get blank or not
		u32 sys_blk = 0;
		u32 emu_blk = 0;

		u32 xcol = 90;
		u32 rowinc = 900 + xcol;
		u32 colinc = 150;
		u32 rowsepar = 50;
		u32 colsepar = 100;

		//if (sd_mount())
		{
			if (sd_file_exists("atmosphere/config/exosphere.ini"))
			{
				void *buf = read_file_string("atmosphere/config/exosphere.ini");
				buffer_blk = buf;
			}
			if (strstr(buffer_blk, "blank_prodinfo_emummc=1") != NULL)
			{
				emu_blk = 1;
			}

			if (strstr(buffer_blk, "blank_prodinfo_sysmmc=1") != NULL)
			{
				sys_blk = 1;
			}

			if (sd_file_exists("emummc/emummc.ini"))
			{
				if (!sd_file_exists("StarDust/emunand_serial.txt"))
				{
					sd_save_2_file("", 0, "StarDust/emunand_serial.txt");
					Incognito("1");
				}

				void *buf = read_file_string("StarDust/emunand_serial.txt");
				emuserial = buf;

				if (strstr(emuserial, "XAW0000000000") != NULL)
				{
					emu_inc = 1;
				}

				if (sd_file_exists("atmosphere/config/exosphere.ini"))
				{
					if (emu_blk == 1)
					{
						create(menus[2], "Icons/sw-on.bmp", 1000, 100, (int (*)(void *))Incognito, (void *)"DBE");
					}
					else
					{
						create(menus[2], "Icons/sw-off.bmp", 1000, 100, (int (*)(void *))Incognito, (void *)"BE");
					}
					//gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("Blank_Prodinfo", 1020, 90, 150, 100, NULL, NULL));
					create_no_bitmap(menus[menuses],"Blank_Prodinfo", 1020, 90);
				}

				//gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("SysNand", 170, 200, 150, 100, NULL, NULL));
				//gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("EmuNand", 970, 200, 150, 100, NULL, NULL));
				create_no_bitmap(menus[menuses],"SysNand",250, 200);
				create_no_bitmap(menus[menuses],"EmuNand",1050, 200);

				rowinc = rowinc - rowsepar;
				colinc = colinc + colsepar;
				gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap(emuserial, rowinc + 100, colinc - 20, 150, 100, NULL, NULL));
				//create_no_bitmap(menus[menuses],emuserial, rowinc + 100, colinc - 20);
				if (!sd_file_exists("prodinfo_emunand.bin"))
					create(menus[2], "Icons/inc3.bmp", rowinc, colinc, (int (*)(void *))Incognito, (void *)"2");
				else
					create(menus[2], "Icons/inc0-off.bmp", rowinc, colinc, NULL, NULL);

				rowinc = rowinc - rowsepar;
				colinc = colinc + colsepar;
				if (emu_inc == 0)
					create(menus[2], "Icons/inc4.bmp", rowinc, colinc, (int (*)(void *))Incognito, (void *)"3");
				else
					create(menus[2], "Icons/inc1-off.bmp", rowinc, colinc, NULL, NULL);

				rowinc = rowinc - rowsepar;
				colinc = colinc + colsepar;
				if (sd_file_exists("prodinfo_emunand.bin"))
					create(menus[2], "Icons/inc5.bmp", rowinc, colinc, (int (*)(void *))Incognito, (void *)"4");
				else
				{
					create(menus[2], "Icons/inc2-off.bmp", rowinc, colinc, NULL, NULL);
					if (emu_inc == 1)
					{
						create_no_bitmap(menus[menuses],"Missing", rowinc + 100, colinc + 100);
						create_no_bitmap(menus[menuses],"/prodinfo_emunand.bin", rowinc + 100, colinc + 120);
						// gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("Missing", rowinc + 100, colinc + 100, 150, 100, NULL, NULL));
						// gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("/prodinfo_emunand.bin", rowinc + 100, colinc + 120, 150, 100, NULL, NULL));
					}
				}
			}
			else
			{
				rowinc = 600;
				rowsepar = 0;
			}

			rowinc = 0 + xcol;
			colinc = 150;
			rowsepar = 50;
			colsepar = 100;
			create(menus[2], "Icons/power.bmp", 80, 500, tool_power_off, NULL);
			if (sd_file_exists("StarDust/sysnand_serial.txt"))
			{

				void *buf = read_file_string("StarDust/sysnand_serial.txt");
				sysserial = buf;
				if (strstr(sysserial, "XAW0000000000") != NULL)
					sys_inc = 1;

				if (sd_file_exists("emummc/emummc.ini"))
				{
					void *buf = read_file_string("StarDust/emunand_serial.txt");
					emuserial = buf;
				}
				else
				{
					rowinc = 550;
					rowsepar = 0;
				}

				if (strstr(emuserial, "XAW0000000000") != NULL)
					emu_inc = 1;

				rowinc = rowinc + rowsepar;
				colinc = colinc + colsepar;

				if (sd_file_exists("atmosphere/config/exosphere.ini"))
				{
					if (sys_blk == 1)
					{
						create(menus[2], "Icons/sw-on.bmp", rowinc - 8 - rowsepar, 100, (int (*)(void *))Incognito, (void *)"DBS");
					}
					else
					{
						create(menus[2], "Icons/sw-off.bmp", rowinc - 8 - rowsepar, 100, (int (*)(void *))Incognito, (void *)"BS");
					}
					gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("Blank_Prodinfo", rowinc + 10 - rowsepar, 90, 150, 100, NULL, NULL));
				}
				gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap(sysserial, rowinc + 100, colinc - 20, 150, 100, NULL, NULL));
				//create_no_bitmap(menus[menuses],sysserial, rowinc + 100, colinc - 20);

				if (!sd_file_exists("prodinfo_sysnand.bin"))
					create(menus[2], "Icons/inc0.bmp", rowinc, colinc, (int (*)(void *))Incognito, (void *)"5");
				else
					create(menus[2], "Icons/inc0-off.bmp", rowinc, colinc, NULL, NULL);

				rowinc = rowinc + rowsepar;
				colinc = colinc + colsepar;
				if (sys_inc == 0)
					create(menus[2], "Icons/inc1.bmp", rowinc, colinc, (int (*)(void *))Incognito, (void *)"6");
				else
					create(menus[2], "Icons/inc1-off.bmp", rowinc, colinc, NULL, NULL);

				rowinc = rowinc + rowsepar;
				colinc = colinc + colsepar;

				if (sd_file_exists("prodinfo_sysnand.bin"))
					create(menus[2], "Icons/inc2.bmp", rowinc, colinc, (int (*)(void *))Incognito, (void *)"7");
				else
				{
					create(menus[2], "Icons/inc2-off.bmp", rowinc, colinc, NULL, NULL);
					if (sys_inc == 1)
					{
						create_no_bitmap(menus[menuses],"Missing", rowinc + 100, colinc + 100);
						create_no_bitmap(menus[menuses],"/prodinfo_sysnand.bin", rowinc + 100, colinc + 120);

						// gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("Missing", rowinc + 100, colinc + 100, 150, 100, NULL, NULL));
						// gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("/prodinfo_sysnand.bin", rowinc + 100, colinc + 120, 150, 100, NULL, NULL));
					}
				}
			}
			else
			{
				gui_menu_append_entry(menus[2], gui_create_menu_entry_no_bitmap("Biskeys error, try again", rowinc + 30, colinc - 10, 150, 100, NULL, NULL));
			}

			//		gui_menu_append_entry(menus[2],gui_create_menu_entry("",theme("Icons/arrowl.bmp"),200, low_icons, 70, 70,(int (*)(void *))tool_Menus, (void*)1));
		}
        static_menu_elements(menus[2]);
        create(menus[2], "Icons/home.bmp", 10, low_icons, (int (*)(void *))tool_Menus, (void *)0);

	}
}

/* Init needed menus for ArgonNX */
void gui_init_argon_menu(void)
{
	// Start Instruction
	gui_init_argon_boot();
	//main menu loop
    while(true){
        change_brightness(0);
        SDStrap();
        if (menus[main_menu]->next_entry == 0)
            pre_load_menus(main_menu, 0);
        
        /* Start menus[0] */
        gui_menu_open(menus[main_menu]);

        
    /*
        gfx_con_setpos( 160, 50);
        gfx_printf( "end off menu \n");
    */
    }
	/* Clear all entries and menus */
	//gui_menu_pool_cleanup();
    return;
}

int static_menu_elements(gui_menu_t *menut)
{
		//create(menu, "/StarDust/skins/xbox/Icons/home.bmp", 10, low_icons, (int (*)(void *))tool_Menus, (void *)0);
/*
	if (main_menu != 1)
		create(menu, "Icons/power.bmp", 605, low_icons, tool_power_off, NULL);

*/
		

	gui_menu_append_entry(menut, gui_create_menu_entry("", NULL, 0, 0, 35, 35, (int (*)(void *))screenshot, NULL));

	//battery
	u32 battPercent = 0;
	max17050_get_property(MAX17050_RepSOC, (int *)&battPercent);
	u32 battimgper = (battPercent >> 8) & 0xFF;
	u32 batY = 5;
	u32 batX = 1200;
	if ((battimgper <= 100) & (battimgper >= 76))
		create(menut, "Icons/bat1.bmp", batX, batY, (int (*)(void *))bat_show, (void *)battimgper);
	if ((battimgper <= 75) & (battimgper >= 51))
		create(menut, "Icons/bat2.bmp", batX, batY, (int (*)(void *))bat_show, (void *)battimgper);
	if ((battimgper <= 50) & (battimgper >= 26))
		create(menut, "Icons/bat3.bmp", batX, batY, (int (*)(void *))bat_show, (void *)battimgper);
	if ((battimgper <= 25) & (battimgper >= 5))
		create(menut, "Icons/bat4.bmp", batX, batY, (int (*)(void *))bat_show, (void *)battimgper);
	if (battimgper <= 4)
		create(menut, "Icons/bat5.bmp", batX, batY, (int (*)(void *))bat_show, (void *)battimgper);
	return 0;
}

int tool_reboot_rcm(void *param)
{
	power_set_state(POWER_OFF);
	return 0;
}

int tool_power_off(void *param)
{
	power_set_state(POWER_OFF);
	return 0;
}

//eject sd card
int tool_extr_rSD(void *param)
{
	SDStrap();
	//gui_menu_pool_cleanup();
	gfx_swap_buffer();
    gfx_clear_grey(0x00);
	change_brightness(0);
	gfx_con.scale = 3;
	gfx_con_setpos( 160, 100);
	gfx_printf( "Ya puedes extraer la SD, Al terminar\n");
	gfx_con_setpos( 230, 130);
	gfx_printf( "Pon la SD y presiona POWER\n\n");
	gfx_con_setpos( 110, 600);
	gfx_printf( "You can now extract the SD, When you finish\n");
	gfx_con_setpos( 230, 630);
	gfx_printf( "Put the SD and press POWER\n");
	gfx_swap_buffer();
	btn_wait_timeout(10000, BTN_POWER);
	display_backlight_brightness(0, 1000);
	/* Clear all entries and menus */
	BootStrapNX();
	return 0;
}

//Save emummc
void save_emummc_cfg(){
	char lbuf[16];
/*
	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);

	itoa(emu_cfg.enabled, lbuf, 10);
	gfx_printf("%s\n",lbuf);
	
	itoa(emu_cfg.sector, lbuf, 16);
	gfx_printf("%s\n",lbuf);
	
	itoa(emu_cfg.id, lbuf, 16);
	gfx_printf("%s\n",lbuf);

	gfx_printf("%s\n",emu_cfg.path);
	gfx_printf("%s\n",emu_cfg.nintendo_path);
    msleep(6000); // Guard against injection VOL+.
	//return;
	// Internal.
*/
	sd_mount();

	FIL fp;

	if (f_open(&fp, "emuMMC/emummc.ini", FA_WRITE | FA_CREATE_ALWAYS) != FR_OK)
		return;

	// Add config entry.
	f_puts("[emummc]\nenabled=", &fp);
	itoa(emu_cfg.enabled, lbuf, 10);
	f_puts(lbuf, &fp);
	
	if(emu_cfg.sector != 0){
		f_puts("\nsector=0x", &fp);
		itoa(emu_cfg.sector, lbuf, 16);
		f_puts(lbuf, &fp);
	}
	
	if(emu_cfg.id != 0){
		f_puts("\nid=0x", &fp);
		itoa(emu_cfg.sector, lbuf, 16);
		f_puts(lbuf, &fp);
	}

	if (emu_cfg.path)
	{
		f_puts("\npath=", &fp);
		f_puts(emu_cfg.path, &fp);
	}

	f_puts("\nnintendo_path=", &fp);
	f_puts(emu_cfg.nintendo_path, &fp);
	
	f_puts("\n", &fp);

	f_close(&fp);
}

//Emu tool
int tool_emu(u32 status)
{
	SDStrap();

	//give sxos emu to ams
	if (status == 33)
	{
		f_unlink("sxos/eMMC");
		change_brightness(1);
		f_mkdir("emuMMC");
		f_mkdir("emuMMC/EF00");
		f_rename("/sxos/emunand", "/emuMMC/EF00/eMMC");
		f_rename("/Emutendo", "/emuMMC/EF00/Nintendo");

		f_rename("emuMMC/EF00/eMMC/boot0.bin", "emuMMC/EF00/eMMC/boot0");
		f_rename("emuMMC/EF00/eMMC/boot1.bin", "emuMMC/EF00/eMMC/boot1");

		f_rename("emuMMC/EF00/eMMC/full.00.bin", "emuMMC/EF00/eMMC/00");
		f_rename("emuMMC/EF00/eMMC/full.01.bin", "emuMMC/EF00/eMMC/01");
		f_rename("emuMMC/EF00/eMMC/full.02.bin", "emuMMC/EF00/eMMC/02");
		f_rename("emuMMC/EF00/eMMC/full.03.bin", "emuMMC/EF00/eMMC/03");
		f_rename("emuMMC/EF00/eMMC/full.04.bin", "emuMMC/EF00/eMMC/04");
		f_rename("emuMMC/EF00/eMMC/full.05.bin", "emuMMC/EF00/eMMC/05");
		f_rename("emuMMC/EF00/eMMC/full.06.bin", "emuMMC/EF00/eMMC/06");
		f_rename("emuMMC/EF00/eMMC/full.07.bin", "emuMMC/EF00/eMMC/07");
		f_rename("emuMMC/EF00/eMMC/full.08.bin", "emuMMC/EF00/eMMC/08");
		f_rename("emuMMC/EF00/eMMC/full.09.bin", "emuMMC/EF00/eMMC/09");
		f_rename("emuMMC/EF00/eMMC/full.10.bin", "emuMMC/EF00/eMMC/10");
		f_rename("emuMMC/EF00/eMMC/full.11.bin", "emuMMC/EF00/eMMC/11");
		f_rename("emuMMC/EF00/eMMC/full.12.bin", "emuMMC/EF00/eMMC/12");
		f_rename("emuMMC/EF00/eMMC/full.13.bin", "emuMMC/EF00/eMMC/13");
		f_rename("emuMMC/EF00/eMMC/full.14.bin", "emuMMC/EF00/eMMC/14");
		if (sd_file_exists("emuMMC/EF00/eMMC/00") & sd_file_exists("emuMMC/EF00/eMMC/boot0") & sd_file_exists("emuMMC/EF00/eMMC/boot1"))
		{
			f_unlink("emummc/emummc.bak");
			f_rename("emummc/emummc.ini", "emummc/emummc.bak");
			f_mkdir("emummc");
			FIL fp;
			f_open(&fp, "emummc/emummc.ini", FA_WRITE | FA_CREATE_ALWAYS);
			f_puts("[emummc]\n", &fp);
			f_puts("enabled=1\n", &fp);
			f_puts("sector=0x0\n", &fp);
			f_puts("path=emuMMC/EF00\n", &fp);
			f_puts("id=0x0000\n", &fp);
			f_puts("nintendo_path=emuMMC/EF00/Nintendo\n", &fp);
			f_close(&fp);
			sd_save_2_file("", 0, "emuMMC/EF00/file_based");
			pre_load_menus(1, 1);
		}
	}

	//return emunand to sxos
	if (status == 66)
	{
		f_unlink("sxos/emunand");
		change_brightness(1);
		f_rename("/emuMMC/EF00/eMMC", "/sxos/emunand");

		f_rename("sxos/emunand/boot0", "sxos/emunand/boot0.bin");
		f_rename("sxos/emunand/boot1", "sxos/emunand/boot1.bin");

		f_rename("sxos/emunand/00", "sxos/emunand/full.00.bin");
		f_rename("sxos/emunand/01", "sxos/emunand/full.01.bin");
		f_rename("sxos/emunand/02", "sxos/emunand/full.02.bin");
		f_rename("sxos/emunand/03", "sxos/emunand/full.03.bin");
		f_rename("sxos/emunand/04", "sxos/emunand/full.04.bin");
		f_rename("sxos/emunand/05", "sxos/emunand/full.05.bin");
		f_rename("sxos/emunand/06", "sxos/emunand/full.06.bin");
		f_rename("sxos/emunand/07", "sxos/emunand/full.07.bin");
		f_rename("sxos/emunand/08", "sxos/emunand/full.08.bin");
		f_rename("sxos/emunand/09", "sxos/emunand/full.09.bin");
		f_rename("sxos/emunand/10", "sxos/emunand/full.10.bin");
		f_rename("sxos/emunand/11", "sxos/emunand/full.11.bin");
		f_rename("sxos/emunand/12", "sxos/emunand/full.12.bin");
		f_rename("sxos/emunand/13", "sxos/emunand/full.13.bin");
		f_rename("sxos/emunand/14", "sxos/emunand/full.14.bin");
		f_rename("/emuMMC/EF00/Nintendo", "/Emutendo");
		if (sd_file_exists("sxos/emunand/full.00.bin") & sd_file_exists("sxos/emunand/boot0.bin") & sd_file_exists("sxos/emunand/boot1.bin"))
		{
			f_unlink("emummc/emummc.ini");
			f_rename("emummc/emummc.bak", "emummc/emummc.ini");
			f_unlink("emuMMC/EF00/file_based");
			f_unlink("/emuMMC/EF00");
			f_unlink("emummc");
			emu_cfg.enabled=0;
			pre_load_menus(1, 1);
		}

	}

	//link sxos hide partition to ams
	if (status == 99)
	{
		change_brightness(1);
		f_mkdir("emummc");
		FIL fp;
		f_open(&fp, "emummc/emummc.ini", FA_WRITE | FA_CREATE_ALWAYS);
		f_puts("[emummc]\n", &fp);
		f_puts("enabled=1\n", &fp);
		f_puts("sector=0x2\n", &fp);
		f_puts("nintendo_path=Emutendo\n", &fp);
		f_close(&fp);
		emummc_load_cfg();
		pre_load_menus(1, 1);
	}

	if (status == 0 || status == 1 )
	{
		emu_cfg.enabled=status;
		save_emummc_cfg();
		pre_load_menus(0, 0);
	}

	return 1;
}

int tool_Menus(u32 param)
{
	SDStrap();
	//set menu number
	main_menu = param;
	//gui_menu_pool_cleanup();
	return 1;
}

int tool_servises_on(char *title)
{
	SDStrap();
	change_brightness(1);
    char path[100];
	//char *path = (char *)malloc(256);
	if (isAMS)
	{
        s_printf(path, "atmosphere/contents/%s/flags", title);
	}
	else
	{
        s_printf(path, "sxos/titles/%s/flags", title);
	}
    f_mkdir(path);
    s_printf(path, "%s/boot2.flag", path);
    sd_save_2_file("", 0, path);
	//pre_load_menus(1, 0);
    return 1;
}

int tool_servises_off(char *title)
{
	SDStrap();
	change_brightness(1);
    char path[100];
	//char *path = (char *)malloc(256);
	if (isAMS)
	{
        s_printf(path, "atmosphere/contents/%s/flags", title);
	}
	else
	{
        s_printf(path, "sxos/titles/%s/flags", title);
	}
    s_printf(path, "%s/boot2.flag", path);
    f_unlink(path);
	//pre_load_menus(1, 0);
    return 1;
}

//Themes ON
int tool_Themes_on(char *cfw)
{
	SDStrap();
	char path[100];
    
	if (strstr(cfw, "sxos") != NULL)
	{
		moverall("/sxos/titles/0100000000001000/sfmor", "/sxos/titles/0100000000001000/romfs", "*", "");
        s_printf(path, "%s/titles/0100000000001000/fsmitm.flag", cfw);
		sd_save_2_file("", 0, path);
	}

	if (strstr(cfw, "Profile") != NULL)
	{
		if (isAMS)
			f_move("/atmosphere/contents/0100000000001013/exefs.off", "/atmosphere/contents/0100000000001013/exefs.nsp");
		else
			f_move("/sxos/titles/0100000000001013/exefs.off", "/sxos/titles/0100000000001013/exefs.nsp");
	}

	if (strstr(cfw, "atmosphere") != NULL)
	{
		moverall("/atmosphere/contents/0100000000001000/sfmor", "/atmosphere/contents/0100000000001000/romfs", "*", "");
		//		copyarall("/atmosphere/contents/0100000000001000/sfmor", "/atmosphere/contents/0100000000001000/romfs", "*","");
		sd_save_2_file("", 0, "atmosphere/contents/0100000000001000/fsmitm.flag");
	}
	pre_load_menus(1, 0);
    return 1;
}

//Themes OFF
int tool_Themes_off(char *cfw)
{
	SDStrap();
	if (strstr(cfw, "sxos") != NULL)
	{
		moverall("/sxos/titles/0100000000001000/romfs", "/sxos/titles/0100000000001000/sfmor", "*", "");
		f_unlink("/sxos/titles/0100000000001000/fsmitm.flag");
	}

	if (strstr(cfw, "Profile") != NULL)
	{
		if (isAMS)
			f_move("/atmosphere/contents/0100000000001013/exefs.nsp", "/atmosphere/contents/0100000000001013/exefs.off");
		else
			f_move("/sxos/titles/0100000000001013/exefs.nsp", "/sxos/titles/0100000000001013/exefs.off");
	}

	if (strstr(cfw, "atmosphere") != NULL)
	{
		//			deleteall("/atmosphere/contents/0100000000001000/romfs", "*", "");
		moverall("/atmosphere/contents/0100000000001000/romfs", "/atmosphere/contents/0100000000001000/sfmor", "*", "");
		f_unlink("atmosphere/contents/0100000000001000/fsmitm.flag");
	}
	pre_load_menus(1, 0);
    return 1;
}

//safe boot
int tool_menu_rem(void *param)
{

    printerCU("", "SafeBoot", 0);
	SDStrap();
	change_brightness(1);
	f_unlink("/atmosphere/contents/0100000000001000/fsmitm.flag");
	f_unlink("/atmosphere/contents/0100000000001000/romfs_metadata.bin");
	deleteall("/atmosphere/contents/0100000000001000/romfs/lyt", "*", "Delete 0100000000001000");
	f_unlink("/SXOS/titles/0100000000001000/fsmitm.flag");
	f_unlink("/sxos/titles/0100000000001000/romfs_metadata.bin");
	f_unlink("/sxos/titles/0100000000000034-OFF/exefs.nsp");
	f_unlink("/sxos/titles/0100000000000034-OFF");
	f_rename("/sxos/titles/0100000000000034", "/sxos/titles/0100000000000034-OFF");
	f_unlink("/sxos/titles/0100000000000034/exefs.nsp");
	//this is a panic option so i will disable the servises also
	char *folder_ams = listfol("/atmosphere/contents", "*", true);
	u32 r = 0;
	while (folder_ams[r * 256])
	{
		if (strlen(&folder_ams[r * 256]) <= 100)
		{
			char source_ams[100];
            s_printf(source_ams, "/atmosphere/contents/%s/flags/boot2.flag",&folder_ams[r * 256]);
			f_unlink(source_ams);
		}
		r++;
	}
    free(folder_ams);
	char *folder_sxos = listfol("/sxos/titles", "*", true);
	u32 e = 0;
	while (folder_sxos[e * 256])
	{
		if (strlen(&folder_sxos[e * 256]) <= 100)
		{
			char source_sxos[100];
            s_printf(source_sxos, "/sxos/titles/%s/flags/boot2.flag",&folder_sxos[r * 256]);
			f_unlink(source_sxos);
		}
		e++;
	}
    free(folder_sxos);

    //Sigpatches
    sd_save_2_file("", 0, "atmosphere/contents/420000000000000B/flags/boot2.flag");
    
    //enable cleanUP
	sd_save_2_file("", 0, "StarDust/flags/IamSafe.flag");
	sd_save_2_file("#Safeboot flag", 13, "fixer.del");
    printerCU("", "", 1);
	launcher("payload.bin");
	return 0;
}

//Themes///// copy
int tool_theme(char *param)
{
	SDStrap();
	change_brightness(1);
	saveTheme(param);
    upd_menus();
	pre_load_menus(1, 1);
	return 1;
}

int bat_show(u32 percent)
{
	gfx_con_setcol( 0xFFF9F9F9, 0xFFCCCCCC, 0xFF191414);
	gfx_con_setpos( 1205, 15);
	gfx_printf( "%k%d%%%k", 0xFF00FF22, percent, 0xFFCCCCCC);

	//      gfx_printf( "%k%d%%%k", 0xFF00FF22,percent,0xFFCCCCCC);
	gfx_con_setcol( 0xFFF9F9F9, 0, 0xFF191414);
	//		for (u32 i =10; i < 720; i = i +20){					gfx_con_setpos( 10, i);	gfx_printf( " %d -------------------------------------------------------------", i);		}
	return 0;
}

int serv_CFW(int cfw)
{
	SDStrap();
	change_brightness(1);
	isAMS = cfw;
	pre_load_menus(1, 0);
    return 1;
}

void serv_display(gui_menu_t *menut, char *titleid, char *name)
{
	SDStrap();
	static u32 servYF = 250;
	static u32 servXF = 160;

	static u32 servX = 160;
	static u32 servY = 250;

	static u32 sepaserv = 70;
	if (servstep == 0){
		servY = servYF;
		servX = servXF;
	}
		
	if (servstep > 9)
		return;

	char path[100];

	if (isAMS)
        s_printf(path, "/atmosphere/contents/%s",titleid);
	else
        s_printf(path, "/sxos/titles/%s",titleid);

    
	char flagpath[100];
    s_printf(flagpath, "%s/flags/boot2.flag",path);
    s_printf(path, "%s/exefs.nsp",path);

	if (!sd_file_exists(path))
		return;
/*
	if (sd_file_exists(flagpath))
	{
		create(menut, "Icons/sw-on.bmp", servX, servY, (int (*)(void *))tool_servises_off, (void *)titleid);
	}
	else
	{
		create(menut, "Icons/sw-off.bmp", servX, servY, (int (*)(void *))tool_servises_on, (void *)titleid);
	}
	gui_menu_append_entry(menut, gui_create_menu_entry_no_bitmap(name, servX + 30, servY + 30, 150, 100, NULL, NULL));
    */
    create_switch(menut, name, "Icons/sw-on.bmp", "Icons/sw-off.bmp", sd_file_exists(flagpath), servX, servY, (int (*)(void *))tool_servises_off, (void *)titleid, (int (*)(void *))tool_servises_on, (void *)titleid);

    
	servstep++;
	if (servstep % 2 == 0)
	{
		servY = servYF;
		servX = servX + 205;
		//servstep = 0;
	}
	else
	{
		servY = servY + sepaserv;
	}
}

int Incognito(char *order){
	SDStrap();
	if (strstr(order, "B") != NULL)
	{

		if (strstr(order, "BS") != NULL)
		{
			buffer_blk = str_replace(buffer_blk, "blank_prodinfo_sysmmc=0", "blank_prodinfo_sysmmc=1");
		}

		if (strstr(order, "DBS") != NULL)
		{
			buffer_blk = str_replace(buffer_blk, "blank_prodinfo_sysmmc=1", "blank_prodinfo_sysmmc=0");
		}

		if (strstr(order, "BE") != NULL)
		{
			buffer_blk = str_replace(buffer_blk, "blank_prodinfo_emummc=0", "blank_prodinfo_emummc=1");
		}

		if (strstr(order, "DBE") != NULL)
		{
			buffer_blk = str_replace(buffer_blk, "blank_prodinfo_emummc=1", "blank_prodinfo_emummc=0");
		}

		u32 size = strlen(buffer_blk) - 1;
		sd_save_2_file(buffer_blk, size, "atmosphere/config/exosphere.ini");
		pre_load_menus(4, 0);
		return 1;
	}

	sd_save_2_file(order, 1, "StarDust/autoboot.inc");
	launcher("StarDust/payloads/Incognito_RCM.bin");
	return 0;
}

int Autoboot(u32 fil){
	SDStrap();
	if (fil == 0)
		f_unlink("StarDust/autobootecho.txt");

	if (fil == 1)
		sd_save_2_file("", 0, "StarDust/autobootecho.txt");
	
	pre_load_menus(1, 0);
	return 1;
};

int AThemes_list(gui_menu_t *menut, u32 gridX, u32 gridY){
	//generate themes Dinamy
	u32 tm_ajust = 90;
	char *folder = listfol("StarDust/skins", "*", true);
	u32 r = 0;
	u32 w = 0;
	while (folder[r * 256])
	{
		if (strlen(&folder[r * 256]) <= 100)
		{
			char *source_fol = (char *)malloc(256);
			strcpy(source_fol, "skins/");
			strcat(source_fol, &folder[r * 256]);
			strcat(source_fol, "/");
			char *source_icon = (char *)malloc(256);
			strcpy(source_icon, "/StarDust/");
			strcat(source_icon, source_fol);
			strcat(source_icon, "icon.bmp");
			if (sd_file_exists(source_icon))
			{
				gui_menu_append_entry(menut, gui_create_menu_entry("", sd_4_file_read2(source_icon), gridX, gridY, 70, 70, (int (*)(void *))tool_theme, (void *)source_fol));
				gridX = gridX + tm_ajust;
			}
            free(source_fol);
            free(source_icon);
			w++;
		}
		if (w == 11)
			break;
		r++;
	}
    free(folder);
	return 0;
}

int medislay(char *flags){
	SDStrap();
	change_brightness(0);
	if (sd_file_exists("StarDust/flags/b50.flag"))
		f_unlink("StarDust/flags/b50.flag");
	else
		sd_save_2_file("", 0, "StarDust/flags/b50.flag");
	//pre_load_menus(1, 0);
    change_brightness(0);
    return 1;
}
/**/

int uLaunch(u32 fil)
{
	SDStrap();
	if (fil == 0)
	{
		f_unlink("/atmosphere/contents/01008BB00013C000/exefs.nsp");
		f_unlink("/atmosphere/contents/010000000000100B/exefs.nsp");
		f_unlink("/atmosphere/contents/010000000000100B/fsmitm.flag");
		f_unlink("/atmosphere/contents/010000000000100B/romfs.bin");
		f_unlink("/atmosphere/contents/0100000000001000/exefs.nsp");
		f_unlink("/atmosphere/contents/0100000000001001/exefs.nsp");

		f_unlink("/sxos/titles/titles/01008BB00013C000/exefs.nsp");
		f_unlink("/sxos/titles/010000000000100B/exefs.nsp");
		f_unlink("/sxos/titles/010000000000100B/fsmitm.flag");
		f_unlink("/sxos/titles/010000000000100B/romfs.bin");
		f_unlink("/sxos/titles/0100000000001000/exefs.nsp");
		f_unlink("/sxos/titles/0100000000001001/exefs.nsp");
		f_unlink("StarDust/flags/ulaunch.flag");
	}

	if (fil == 1)
	{
		copyarall("/StarDust/ulaunch/SdOut/ulaunch", "/ulaunch", "*", "Installing ulaunch");
		copyarall("/StarDust/ulaunch/SdOut/atmosphere", "/atmosphere", "*", "Installing ulaunch");
		//		copyarall("/StarDust/ulaunch/titles", "/ReiNX/titles", "*","Installing ulaunch");
		//copyarall("/StarDust/ulaunch/titles", "/sxos/titles", "*", "Installing ulaunch");
		f_mkdir("StarDust/flags");
		sd_save_2_file("", 0, "StarDust/flags/ulaunch.flag");
	}
	printerCU("", "", 1); //flush print
	pre_load_menus(1, 0);
	return 1;
}

int launcher(char *path){
	SDStrap();
	gfx_swap_buffer();
	display_backlight_brightness(30, 1000);	

	//u32 bootS = sd_file_size("StarDust/boot.dat");

	//Atmosphere
    if(strstr(path,"fusee") != NULL)
    {
		u32 bootF = sd_file_size("StarDust/boot_forwarder.dat");
		u32 bootR = sd_file_size("boot.dat");
		if (sd_file_exists ("StarDust/autobootecho.txt")||(btn_read() & BTN_VOL_UP))
		sd_save_2_file("fusee", 10, "StarDust/autobootecho.txt");
	
        if (bootF < bootR){
            copyfile("StarDust/boot_forwarder.dat","boot.dat");
        }
		
		//Reboot to payload
		u32 pay1 = sd_file_size("payload.bin");
		u32 pay2 = sd_file_size("atmosphere/reboot_payload.bin");
		if (pay1 != pay2){
			copyfile("payload.bin","atmosphere/reboot_payload.bin");
		}
		
		if(!syspatch)
			_cfw(emu_cfg.enabled);
    }
/*
	//SxOS
    if((strstr(path,"sxos") != NULL) || (strstr(path,"SXOS") != NULL))
    {
		gfx_con.scale = 3;


		if (bootS != bootR){
			gfx_con_setpos( 370, 350);
			gfx_printf( "Loading Boot.dat\n",bootS,bootR);
			gfx_swap_buffer();
			copyfile("StarDust/boot.dat","boot.dat");
		}
			
		if (sd_file_exists ("StarDust/autobootecho.txt")||(btn_read() & BTN_VOL_UP))
		sd_save_2_file("SXOS", 4, "StarDust/autobootecho.txt");
	}
*/
    if(strstr(path,"android") != NULL)
    {
		if (sd_file_exists ("StarDust/autobootecho.txt")||(btn_read() & BTN_VOL_UP))
		sd_save_2_file("TWRP", 4, "StarDust/autobootecho.txt");		
    }
    if(strstr(path,"ubuntu") != NULL)
    {
		if (sd_file_exists ("StarDust/autobootecho.txt")||(btn_read() & BTN_VOL_UP))
		sd_save_2_file("Ubuntu", 6, "StarDust/autobootecho.txt");		
    }
	display_backlight_brightness(10, 1000);
	launch_payload(path);
	return 0;
}


/*
void HBhide(char *folder)
{
	SDStrap();
	char *source_fol = (char *)malloc(256);
	strcpy(source_fol, "/switch");
	strcat(source_fol, "/");
	strcat(source_fol, folder);
	strcat(source_fol, "/hide.flag");
	if (sd_file_exists(folder))
		f_unlink(folder);
	else
		sd_save_2_file("", 0, folder);
	pre_load_menus(5, 0);
}
*/
void change_brightness(u32 type)
{
	SDStrap();
	int bright = 100;
	if (sd_file_exists("StarDust/flags/b50.flag"))
		bright = 10;

	if (type == 0)
		display_backlight_brightness(bright, 1000);

	if (type == 1)
		display_backlight_brightness(bright / 2, 1000);
}
