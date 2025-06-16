
#define ATM_FATAL_ERR_CTX_ADDR 0x4003E000
#define  ATM_FATAL_MAGIC       0x30454641 // AFE0

#define ATM_EXO_FATAL_ADDR     0x80020000
#define  ATM_EXO_FATAL_SIZE    SZ_128K

#define HOS_PID_BOOT2 0x8

// Atmosphere reboot-to-fatal-error.
typedef struct _atm_fatal_error_ctx
{
	u32 magic;
	u32 error_desc;
	u64 title_id;
	union
	{
		u64 gprs[32];
		struct
		{
			u64 _gprs[29];
			u64 fp;
			u64 lr;
			u64 sp;
		};
	};
	u64 pc;
	u64 module_base;
	u32 pstate;
	u32 afsr0;
	u32 afsr1;
	u32 esr;
	u64 far;
	u64 report_identifier; // Normally just system tick.
	u64 stack_trace_size;
	u64 stack_dump_size;
	u64 stack_trace[0x20];
	u8  stack_dump[0x100];
	u8  tls[0x100];
} atm_fatal_error_ctx;


static const char *get_error_desc(u32 error_desc)
{
	switch (error_desc)
	{
	case 0x100:
		return "IABRT"; // Instruction Abort.
	case 0x101:
		return "DABRT"; // Data Abort.
	case 0x102:
		return "IUA";   // Instruction Unaligned Access.
	case 0x103:
		return "DUA";   // Data Unaligned Access.
	case 0x104:
		return "UDF";   // Undefined Instruction.
	case 0x106:
		return "SYS";   // System Error.
	case 0x301:
		return "SVC";   // Bad arguments or unimplemented SVC.
	case 0xF00:
		return "KRNL";  // Kernel panic.
	case 0xFFD:
		return "SO";    // Stack Overflow.
	case 0xFFE:
		return "std::abort";
	default:
		return "UNK";
	}
}
void fix_errors()
{
	volatile atm_fatal_error_ctx *rpt = (atm_fatal_error_ctx *)ATM_FATAL_ERR_CTX_ADDR;
    bool fixed = false;
    
	// Mask magic to maintain compatibility with any AFE version, thanks to additive struct members.
	if ((rpt->magic & 0xF0FFFFFF) != ATM_FATAL_MAGIC)
		return;

	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);

	WPRINTF("Panic occurred while running Atmosphere.\n\n");
	WPRINTFARGS("Title ID: %08X%08X", (u32)((u64)rpt->title_id >> 32), (u32)rpt->title_id);
	WPRINTFARGS("Error:    %s (0x%x)\n", get_error_desc(rpt->error_desc), rpt->error_desc);

	// Check if mixed atmosphere sysmodules.
	if ((u32)rpt->title_id == HOS_PID_BOOT2)
		WPRINTF("Mismatched Atmosphere files?\n");

    char titleid[256];
    s_printf(titleid, "%08X%08X", (u32)((u64)rpt->title_id >> 32), (u32)rpt->title_id);
    
    char path[256];
    
    //fix error ob dnmt
    if(strstr(titleid, "010000000000000D") != NULL){
        s_printf(path, "/atmosphere/contents/%s/", titleid);
        f_unlink(path);
        deleteall(path, "*", "");
        fixed = true;
    }
    
    //fix error with mission control
    if(strstr(titleid, "010000000000BD00") != NULL){
        s_printf(path, "/atmosphere/contents/%s/flags/boot2.flag", titleid);
        f_unlink(path);
        fixed = true;
    }

	// Change magic to invalid, to prevent double-display of error/bootlooping.
	rpt->magic = 0;
    if(fixed) {
        gfx_printf("\n\nFixed.....\n");
        msleep(800);
        gfx_printf("\n\nRebooting.....\n");
        msleep(1800);
        return;
    }
    
	gfx_printf("\n\nPress POWER to continue.\n");

	display_backlight_brightness(100, 1000);
	msleep(1000);

	while (!(btn_wait() & BTN_POWER))
		;

	display_backlight_brightness(0, 1000);
	gfx_con_setpos(0, 0);
}


//mod payload
void _launch_option(char *path, u8 opt)
{
	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);

	FIL fp;
	if (f_open(&fp, path, FA_READ))
	{
		gfx_con.mute = false;
		EPRINTFARGS("Payload file is missing!\n(%s)", path);

		goto out;
	}

	// Read and copy the payload to our chosen address
	void *buf;
	u32 size = f_size(&fp);

	if (size < 0x30000)
		buf = (void *)RCM_PAYLOAD_ADDR;
	else
	{
		coreboot_addr = (void *)(COREBOOT_END_ADDR - size);
		buf = coreboot_addr;
		if (h_cfg.t210b01)
		{
			f_close(&fp);

			gfx_con.mute = false;
			EPRINTF("Coreboot not allowed on Mariko!");

			goto out;
		}
	}

	if (f_read(&fp, buf, size, NULL))
	{
		f_close(&fp);

		goto out;
	}

	f_close(&fp);


	sd_end();

	if (size < 0x30000)
	{
		// Enable boot storage.
		b_cfg.boot_cfg |= BOOT_CFG_AUTOBOOT_EN;
		b_cfg.autoboot      = opt;
		b_cfg.autoboot_list = 0;

        memcpy((u8 *)(RCM_PAYLOAD_ADDR + PATCHED_RELOC_SZ), &b_cfg, sizeof(boot_cfg_t)); // Transfer boot cfg.

		_reloc_patcher(PATCHED_RELOC_ENTRY, EXT_PAYLOAD_ADDR, ALIGN(size, 0x10));

		hw_deinit(false, byte_swap_32(*(u32 *)(buf + size - sizeof(u32))));
	}
	else
	{
		goto out;
	}

	//void (*update_ptr)()      = (void *)RCM_PAYLOAD_ADDR;
	void (*ext_payload_ptr)() = (void *)EXT_PAYLOAD_ADDR;

	// Launch our payload.
    // Set updated flag to skip check on launch.
    EMC(EMC_SCRATCH0) |= EMC_HEKA_UPD;
    // Some cards (Sandisk U1), do not like a fast power cycle. Wait min 100ms.
    sdmmc_storage_init_wait_sd();

    (*ext_payload_ptr)();


out:
	gfx_con.mute = false;
	EPRINTF("Failed to launch payload!");
}



/*
extern u32 _find_section_name(char *lbuf, u32 lblen, char schar);
extern ini_sec_t *_ini_create_section(link_t *dst, ini_sec_t *csec, char *name, u8 type);

void cfg_add(ini_sec_t *cfg_sec, const char *line) {
	
	//if (!cfg_sec) cfg_sec = _ini_create_section(NULL, NULL, NULL, INI_CHOICE);

    char lbuf[512];
    // Copiar la línea a lbuf, asegurándote de no exceder el tamaño de lbuf.
    strncpy(lbuf, line, sizeof(lbuf) - 1);
    // Asegurarse de que la cadena esté terminada en nulo
    lbuf[sizeof(lbuf) - 1] = '\0';

    // Encuentra el índice del separador '=' en lbuf.
    u32 i = _find_section_name(lbuf, strlen(lbuf), '=');

    // Calcular los tamaños de clave y valor.
    u32 klen  = strlen(&lbuf[0]) + 1;
    u32 vlen  = strlen(&lbuf[i + 1]) + 1;
    
    // Asignar memoria para ini_kv_t, la clave y el valor.
    char *buf = zalloc(sizeof(ini_kv_t) + klen + vlen);

    ini_kv_t *kv = (ini_kv_t *)buf;
    buf += sizeof(ini_kv_t);

    // Copiar la clave y el valor en el buffer asignado.
    kv->key = strcpy_ns(buf, &lbuf[0]);
    buf += klen;
    kv->val = strcpy_ns(buf, &lbuf[i + 1]);

    // Añadir la clave-valor a la lista en cfg_sec.
    list_append(&cfg_sec->kvs, &kv->link);
}

void _cfg_launch(ini_sec_t *cfg_sec)
{
	display_backlight_brightness(0, 1000);

	gfx_clear_grey(0x1B);
	gfx_con_setpos(0, 0);
	sd_mount();
	
	if (!cfg_sec)
	{
		gfx_printf("\ncfg_sec NULL ?...\n");
		msleep(2000);
	}
	hos_launch(cfg_sec);
    
    {
		gfx_printf("\nStock Failed to Launch\n");
		gfx_printf("\nPress any key...\n");
	}


	//sd_end();
	h_cfg.emummc_force_disable = false;
	display_backlight_brightness(h_cfg.backlight, 1000);
	msleep(2000);
	btn_wait();
}
*/
void _cfw(bool emummc)
{
    /*
	ini_sec_t *cfg_sec = NULL;
    char head[100]; strcpy(head, "Atmosphere CFW");
	cfg_sec = _ini_create_section(NULL, NULL, head, INI_CHOICE);
	cfg_add(cfg_sec, "pkg3=atmosphere/package3");
	cfg_add(cfg_sec, "kernelprocid=1");
	//cfg_add(cfg_sec, "exofatal=/payload.bin");
	cfg_add(cfg_sec, "kip1patch=nosigchk");
	if(emummc) cfg_add(cfg_sec, "emummcforce=1");
	h_cfg.emummc_force_disable = !emummc;
	_cfg_launch(cfg_sec);
    
    */
    if(!emummc)
        _launch_option("/StarDust/payloads/hekate.bin",3);
    else
        _launch_option("/StarDust/payloads/hekate.bin",4);

        
}

void _stock_launch()
{
    _launch_option("/StarDust/payloads/hekate.bin",2);
    /*
	ini_sec_t *cfg_sec = NULL;
	cfg_sec = _ini_create_section(NULL, NULL, NULL, INI_CHOICE);
	cfg_add(cfg_sec, "pkg3=atmosphere/package3");
	cfg_add(cfg_sec, "stock=1");
	h_cfg.emummc_force_disable = true;
	_cfg_launch(cfg_sec);
    */
}
