#include "../main.c"
#include "tools/fs_utils.h"
#include "tools/touch2.h"
#include "gui/gui_argon_menu.h"
#include "tools/update.h"
#include "tools/tools.h"
#include "input/touch.h"
#include "Land.h"
u8 battery_icons_blz[] = {};
int REVI_VERSION = 20;

int llaunch_payload(char *path)
{
    _launch_payload(path, false, true);
	return 1;
}

void takeoff(){
    //gfx_clear_buffer();
    display_backlight_brightness(h_cfg.backlight, 1000);
	//h_cfg.updater2p = 0;
	/* Mount Sd card */
	if (sd_mount())
	{
        minerva_change_freq(FREQ_1600);//moverall("/amame", "/atmosphere", "*", "test");        cyper("payload.bin","payload.enc");cyper("payload.enc","payload_out.bin");msleep(5000);power_set_state(POWER_OFF);
        
		//Update And Clean
		update_std();
		clean_up();
        
        //Check for errors
        CheckLogs("/atmosphere/crash_reports");

		/* Cofigure touch input */
		touch_power_on();
        emummc_load_cfg();
        
        //sd_save_2_file("", 0, "StarDust/flags/TAKEOVER.flag");
        if (sd_file_exists("StarDust/flags/TAKEOVER.flag"))
        {
            f_unlink("StarDust/flags/TAKEOVER.flag");

        	u8 *buf = zalloc(0x200);
			is_ipl_updated(buf, 0, "payload.bin", true);
			//is_ipl_updated(buf, 0, "atmosphere/reboot_payload.bin", true);
			free(buf);
        }

		//summon Menu
		gui_init_argon_menu();
        
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

// Alternative main() function
void ipl_main2()
{
	// Override DRAM ID if needed.
	if (ipl_ver.rcfg.rsvd_flags & RSVD_FLAG_DRAM_8GB)
		fuse_force_8gb_dramid();

	// Do initial HW configuration. This is compatible with consecutive reruns without a reset.
	hw_init();

	// Pivot the stack under IPL. (Only max 4KB is needed).
	pivot_stack(IPL_LOAD_ADDR);

	// Place heap at a place outside of L4T/HOS configuration and binaries.
	heap_init((void *)IPL_HEAP_START);

#ifdef DEBUG_UART_PORT
	uart_send(DEBUG_UART_PORT, (u8 *)"hekate: Hello!\r\n", 16);
	uart_wait_xfer(DEBUG_UART_PORT, UART_TX_IDLE);
#endif

	// Set bootloader's default configuration.
	set_default_configuration();

	// Check if battery is enough.
	_check_low_battery();

	// Prep RTC regs for read. Needed for T210B01 R2C.
	max77620_rtc_prep_read();

	// Initialize display.
	display_init();

	// Overclock BPMP.
	bpmp_clk_rate_set(h_cfg.t210b01 ? ipl_ver.rcfg.bclk_t210b01 : ipl_ver.rcfg.bclk_t210);

	// Mount SD Card.
	h_cfg.errors |= !sd_mount() ? ERR_SD_BOOT_EN : 0;

	// Check if watchdog was fired previously.
	if (watchdog_fired())
		goto skip_lp0_minerva_config;

	// Enable watchdog protection to avoid SD corruption based hanging in LP0/Minerva config.
	watchdog_start(5000000 / 2, TIMER_FIQENABL_EN); // 5 seconds.

	// Save sdram lp0 config.
	void *sdram_params = h_cfg.t210b01 ? sdram_get_params_t210b01() : sdram_get_params_patched();
	if (!ianos_loader("bootloader/sys/libsys_lp0.bso", DRAM_LIB, sdram_params))
		h_cfg.errors |= ERR_LIBSYS_LP0;

	// Train DRAM and switch to max frequency.
	if (minerva_init((minerva_str_t *)&nyx_str->minerva)) //!TODO: Add Tegra210B01 support to minerva.
		h_cfg.errors |= ERR_LIBSYS_MTC;

	// Disable watchdog protection.
	watchdog_end();

skip_lp0_minerva_config:
	// Initialize display window, backlight and gfx console.
	u32 *fb = display_init_window_a_pitch();
	gfx_init_ctxt(fb, 1280, 720, 720);
	gfx_con_init();

	// Initialize backlight PWM.
	display_backlight_pwm_init();
	//display_backlight_brightness(h_cfg.backlight, 1000);

	// Get R2C config from RTC.
	if (h_cfg.t210b01)
		_r2c_get_config_t210b01();

	// Show exceptions, HOS errors, library errors and L4T kernel panics.
	//_show_errors();
    fix_errors();

	// Load saved configuration and auto boot if enabled.
	// if (!(h_cfg.errors & ERR_SD_BOOT_EN))
		// _auto_launch();

    takeoff();

	
	// Failed to launch Argon, unmount SD Card.
	sd_end();

	// Set ram to a freq that doesn't need periodic training.
	minerva_change_freq(FREQ_800);
/*
	while (true)
		tui_do_menu(&menu_top);
*/
	// Halt BPMP if we managed to get out of execution.
	while (true)
		bpmp_halt();
}
