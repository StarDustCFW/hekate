/*
    extra file

*/
int REVI_VERSION = 4;
u32 fusesB()
{
	u32 burntFuses = bit_count(fuse_read_odm(7));
    return burntFuses;
}

void BootStrapNX()
{
    minerva_change_freq(FREQ_204);
    gfx_clear_grey(0x00);
    sd_unmount();
    u32 battPercent = 0;
    u32 letX = 20;
    u32 letY = 380;

    u32 a = 0;
    u32 b = 100;
    u32 c = 20;

    u32 count = 0;
    u32 countfull = 7*5;

    u32 burntFuses = fusesB();
    char* mindowngrade = "";//fusesM(); ToDo
    gfx_clear_grey(0x00);
    while (btn_read() & BTN_POWER);
    display_backlight_brightness(a, 1000);
	while (true)
	{
        
		u32 res = 0, btn = btn_read();
		do
		{
			res = btn_read();
            max17050_get_property(MAX17050_RepSOC, (int *)&battPercent);
            battPercent = (battPercent >> 8) & 0xFF;
			if (battPercent < 5){power_set_state(POWER_OFF);}
			msleep(200);
			if (count>0){
				count--;
				if (count==0){
                    display_backlight_brightness(a, 1000);
                    gfx_clear_grey(0x00);
                } else  if (count<=countfull/3){
                    display_backlight_brightness(c, 1000);
                }
			}
		} while (btn == res);
        gfx_clear_grey(0x00);
		if (res & BTN_POWER) {
			if (btn_read() & BTN_VOL_UP){power_set_state(REBOOT_RCM);}
			
			//if hold power buton then power off
			if (btn_read() & BTN_POWER) msleep(1000);
			if (btn_read() & BTN_POWER) msleep(1000);
			if (btn_read() & BTN_POWER){
                display_backlight_brightness(b, 1000);
				power_set_state(POWER_OFF);
			}
            gfx_con_setpos(1,100);
			if (sd_mount())
			{
                
				_launch_payload("payload.bin",false,false);
				sd_unmount();
				gfx_printf("%kpayload.bin%k missing%k\n",0xFF008F39,0xFFea2f1e,0xFFF9F9F9);
                display_backlight_brightness(b, 1000);
			}else{
				
                gfx_printf("%kSD card Mount failed...%k\n",0xFFea2f1e,0xFFF9F9F9);
			}
			//if hold power buton then power off
			if (btn_read() & BTN_POWER)
			display_backlight_brightness(b, 1000);
			if (btn_read() & BTN_POWER)
			msleep(1000);
			if (btn_read() & BTN_POWER)
			power_set_state(POWER_OFF);
		}

		gfx_con.fntsz = 16;
		gfx_con_setpos(1,1);
		gfx_printf("%k%d%k%k%d%k%k%s%k\n\n",0xFF00FF22, REVI_VERSION ,0xFFCCCCCC, 0XFFEA2F1E, burntFuses ,0xFFCCCCCC ,0XFF331AD8 ,mindowngrade ,0xFFCCCCCC);
//		gfx_con.fntsz = 16;
		gfx_con_setpos(1, 20);
		gfx_con_setcol(0xFF008F39, 0xFF726F68, 0xFF191414);
		gfx_printf("BootStrapNX\n");
		gfx_con_setcol(0xFFF9F9F9, 0, 0xFF191414);
		gfx_con_setpos(1, 38);
		gfx_printf("StarDustCFW\n");
		gfx_con_setpos(1120, 1);
		gfx_printf("%d%%", battPercent);
		
		
		//Draw Footer
		gfx_con.fntsz = 16;
		gfx_con_setpos(letX, letY+250);
		gfx_printf("Press %kPOWER%k To Boot %kpayload.bin%k\n",0xFF331ad8,0xFFF9F9F9,0xFF008F39,0xFFF9F9F9);
		gfx_con_setpos(letX, letY+280);
		gfx_printf("Hold %kVol+ POWER%k To Reboot RCM\n",0xFF331ad8,0xFFF9F9F9);
		gfx_con_setpos(letX, letY+310);
		gfx_printf("Hold %kPOWER%k To Full Power Off\n",0xFF331ad8,0xFFF9F9F9);
		
		display_backlight_brightness(b, 1000);
		//gfx_clear_grey(0x00);
        //msleep(1000);
		count=countfull;
	}
}

//Tools
void printerCU(char *text,const char *title,int clean){
		static char titw[999] = "-.-";
		static char buff[999] = "\0";
		static int count = 0;
		if(strlen(title) <= 0){
			title=titw;
		}

		
		if (clean == 1){
			count = 0;
			strcpy(buff, "\0");
			strcpy(titw, "\0");
            gfx_clear_grey(0x1B);
			return;
		}
		if (clean != 2) gfx_clear_grey(0x1B);
        
		gfx_con.fntsz = 16;
		gfx_con_setpos(10, 5);
		gfx_con_setcol(0xFF008F39, 0xFF726F68, 0xFF191414);
		gfx_printf("%s\n",title);
		gfx_con_setcol(0xFFF9F9F9, 0, 0xFF191414);
		gfx_con.fntsz = 8;
		gfx_printf("%s\n",buff);
		gfx_printf("\n->%s<-\n",text);
		//gfx_swap_buffer(&gfx_ctxt);
		if (clean > 100){msleep(clean);clean=2;}//
		if (clean == 0)
		{
			if(strlen(title) > 0){
				strcpy(titw, "\0");
				strcpy(titw, title);
			}
			if (count > 338)
			{
				count = 0;
				strcpy(buff, "\0");
                gfx_clear_grey(0x1B);

			}
			if(strlen(text) > 0){
				strcat(buff, text);
				strcat(buff, "\n");
				count++;
			}
		}
}
char *str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep (the string to remove)
    int len_with; // length of with (the string to replace rep with)
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    // sanity checks and initialization
    if (!orig || !rep)
        return NULL;
    len_rep = strlen(rep);
    if (len_rep == 0)
        return NULL; // empty rep causes infinite loop during count
    if (!with)
        with = "";
    len_with = strlen(with);

    // count the number of replacements needed
    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}

void strrep(char *orig, char *rep, char *with){
	char *result = str_replace(orig, rep, with);
	memset(orig,0,strlen(orig));
	strcat(orig,result);
}

//Dir list
char *listfol(const char *directory, const char *pattern, bool includeHiddenFiles){
	u8 max_entries = 61;

	int res = 0;
	u32 i = 0, j = 0, k = 0;
	DIR dir;
	static FILINFO fno;
	
	char *dir_entries = (char *)calloc(max_entries, 256);
	char *copy_entries = (char *)calloc(max_entries, 256);
	char *temp = (char *)calloc(1, 256);

	if (!pattern && !f_opendir(&dir, directory))
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno);
			if (res || !fno.fname[0])
				break;
			if (!(fno.fattrib & AM_DIR) && (fno.fname[0] != ':') && (includeHiddenFiles || !(fno.fattrib & AM_HID)))
			{
				memcpy(dir_entries + (k * 256), fno.fname, strlen(fno.fname) + 1);
				k++;
				if (k > (max_entries - 1))
					break;
			}
		}
		f_closedir(&dir);
	}
	else if (pattern && !f_findfirst(&dir, &fno, directory, pattern) && fno.fname[0])
	{
		do
		{
			if ((fno.fattrib & AM_DIR) && (fno.fname[0] != ':') && (includeHiddenFiles || !(fno.fattrib & AM_HID)))
			{
				memcpy(dir_entries + (k * 256), fno.fname, strlen(fno.fname) + 1);
				k++;
				if (k > (max_entries - 1))
					break;
			}
			res = f_findnext(&dir, &fno);
		} while (fno.fname[0] && !res);
		f_closedir(&dir);
	}

	if (!k)
	{
		free(temp);
		free(dir_entries);
		free(copy_entries);

		return NULL;
	}

	// make copy_entries lowercase version of dir_entries
	for(i = 0; i < k; i++) 
	{
		j = i * 256;
		while(dir_entries[j]) 
		{
			copy_entries[j] = dir_entries[j];

			if(dir_entries[j] >= 'A' && dir_entries[j] <= 'Z')
				copy_entries[j] += 32;
			
			j++;
		}
		
		copy_entries[j] = '\0';
	}

	// compare copy_entries but sort dir_entries
	for (i = 0; i < k - 1 ; i++)
	{
		for (j = i + 1; j < k; j++)
		{
			if (strcmp(&copy_entries[i * 256], &copy_entries[j * 256]) > 0) 
			{
				memcpy(temp, &dir_entries[i * 256], strlen(&dir_entries[i * 256]) + 1);
				memcpy(&dir_entries[i * 256], &dir_entries[j * 256], strlen(&dir_entries[j * 256]) + 1);
				memcpy(&dir_entries[j * 256], temp, strlen(temp) + 1);
			}
		}
	}

	free(temp);
	free(copy_entries);

	return dir_entries;
	}

char *listfil(const char *directory, const char *pattern, bool includeHiddenFiles){
	u8 max_entries = 61;

	int res = 0;
	u32 i = 0, j = 0, k = 0;
	DIR dir;
	static FILINFO fno;
	
	char *dir_entries = (char *)calloc(max_entries, 256);
	char *copy_entries = (char *)calloc(max_entries, 256);
	char *temp = (char *)calloc(1, 256);

	if (!pattern && !f_opendir(&dir, directory))
	{
		for (;;)
		{
			res = f_readdir(&dir, &fno);
			if (res || !fno.fname[0])
				break;
			if (!(fno.fattrib & AM_DIR) && (fno.fname[0] != ':') && (includeHiddenFiles || !(fno.fattrib & AM_HID)))
			{
				memcpy(dir_entries + (k * 256), fno.fname, strlen(fno.fname) + 1);
				k++;
				if (k > (max_entries - 1))
					break;
			}
		}
		f_closedir(&dir);
	}
	else if (pattern && !f_findfirst(&dir, &fno, directory, pattern) && fno.fname[0])
	{
		do
		{
			if (!(fno.fattrib & AM_DIR) && (fno.fname[0] != ':') && (includeHiddenFiles || !(fno.fattrib & AM_HID)))
			{
				memcpy(dir_entries + (k * 256), fno.fname, strlen(fno.fname) + 1);
				k++;
				if (k > (max_entries - 1))
					break;
			}
			res = f_findnext(&dir, &fno);
		} while (fno.fname[0] && !res);
		f_closedir(&dir);
	}

	if (!k)
	{
		free(temp);
		free(dir_entries);
		free(copy_entries);

		return NULL;
	}

	// make copy_entries lowercase version of dir_entries
	for(i = 0; i < k; i++) 
	{
		j = i * 256;
		while(dir_entries[j]) 
		{
			copy_entries[j] = dir_entries[j];

			if(dir_entries[j] >= 'A' && dir_entries[j] <= 'Z')
				copy_entries[j] += 32;
			
			j++;
		}
		
		copy_entries[j] = '\0';
	}

	// compare copy_entries but sort dir_entries
	for (i = 0; i < k - 1 ; i++)
	{
		for (j = i + 1; j < k; j++)
		{
			if (strcmp(&copy_entries[i * 256], &copy_entries[j * 256]) > 0) 
			{
				memcpy(temp, &dir_entries[i * 256], strlen(&dir_entries[i * 256]) + 1);
				memcpy(&dir_entries[i * 256], &dir_entries[j * 256], strlen(&dir_entries[j * 256]) + 1);
				memcpy(&dir_entries[j * 256], temp, strlen(temp) + 1);
			}
		}
	}

	free(temp);
	free(copy_entries);

	return dir_entries;
}

//Fs Utils
void *sd_file_read2(const char *path){
	FIL fp;
	if (f_open(&fp, path, FA_READ) != FR_OK)
		return NULL;

	u32 size = f_size(&fp);
	void *buf = malloc(size);

	u8 *ptr = buf;
	while (size > 0)
	{
		u32 rsize = MIN(size, 512 * 512);
		if (f_read(&fp, ptr, rsize, NULL) != FR_OK)
		{
			free(buf);
			return NULL;
		}

		ptr += rsize;
		size -= rsize;
	}

	f_close(&fp);

	return buf;
}

char *read_file_string(char *path){
	FIL file;
	f_open(&file, path, FA_READ);
	FILINFO stats;

	f_stat(path, &stats);
	unsigned int size = stats.fsize;

	char *buff = malloc(size + 1);
	buff[size] = '\0';
	f_read(&file, buff, size, &size);

	f_close(&file);
	return buff;
}
/*
int sd_save_to_file(void *buf, u32 size, const char *filename)
{
	FIL fp;
	u32 res = 0;
	res = f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE);
	if (res)
	{
        gfx_printf("%kError (%d) creating file\n%s.\n%k\n", 0xFFFFDD00, res, filename, 0xFFCCCCCC);
		return 1;
	}

	f_sync(&fp);
	f_write(&fp, buf, size, NULL);
	f_close(&fp);

	return 0;
}
*/
bool sd_file_exists(const char* filename){
    FRESULT fr;
    FILINFO fno;
    fr = f_stat(filename, &fno);
    switch (fr) {
    case FR_OK:
	return true;
        break;

    case FR_NO_FILE:
        return false;
        break;

    default:
        return false;
    }
}

bool sd_file_size(char *path){
	FIL fp;
	if (f_open(&fp, path, FA_READ) != FR_OK)
		return 0;

	u32 size = f_size(&fp);
	f_close(&fp);
	return size;
}

void copyfile(const char* source, const char* target){
    FIL fp;
    if (f_open(&fp, source, FA_READ) != FR_OK)
    {
        gfx_printf("file %s mising\n",source);
        //msleep(3000);
	}else{
        u32 size = f_size(&fp);
        f_close(&fp);
        sd_save_to_file(sd_file_read2(source),size,target);
	}
}

void copyfileparam(char* param, char* source, char* target){
	char* path = (char*)malloc(256);
    strcpy(path, param);
    strcat(path, "/");
    strcat(path, source);
/*		
		gfx_con.fntsz = 16;
        gfx_con_setpos(15, 50);
		gfx_printf("--------------\n",path);
		gfx_printf("copy %s %s\n",path ,target);
*/		
    FIL fp;
    if (f_open(&fp, path, FA_READ) != FR_OK)
    {
		gfx_printf("file %s mising\n",path);
		msleep(3000);
    }else{
        u32 size = f_size(&fp);
        f_close(&fp);
        sd_save_to_file(sd_file_read2(path),size,target);
	}
}

void copy_folder(char* sourse_folder, char* dest_folder){
    if (!sd_file_exists(sourse_folder)) return;
    char* Files = listfil(sourse_folder, "*", true);
    u32 i = 0;
    while(Files[i * 256])
    {
        char* source_file = (char*)malloc(256);
        if(strlen(&Files[i * 256]) <= 100){	
            strcpy(source_file, sourse_folder);
            strcat(source_file, "/");
            strcat(source_file, &Files[i * 256]);
        }
        char* dest_file = (char*)malloc(256);
        if(strlen(&Files[i * 256]) <= 100){		
            strcpy(dest_file, dest_folder);
            strcat(dest_file, "/");
            strcat(dest_file, &Files[i * 256]);
            gfx_con_setpos(10, 90);
            gfx_printf("\ncopy %s to %s\n",source_file,dest_file);
            copyfile(source_file,dest_file);//action
        }
        i++;
    }
}

//move code
void moverall(char* directory, char* destdir, char* filet, char* coment){
if (!sd_file_exists(directory)) return;
char* files = listfil(directory, filet, true);
char* folder = listfol(directory, "*", true);
f_mkdir(destdir);
    u32 i = 0;
    while(files[i * 256])
    {
        char* sourcefile = (char*)malloc(256);
			if(strlen(&files[i * 256]) <= 100){			
			strcpy(sourcefile, "\0");
			strcat(sourcefile, directory);
			strcat(sourcefile, "/");
			strcat(sourcefile, &files[i * 256]);
			
        char* destfile = (char*)malloc(256);
			strcpy(destfile, "\0");
			strcat(destfile, destdir);
			strcat(destfile, "/");
			strcat(destfile, &files[i * 256]);
			if(strlen(coment) > 0){
				printerCU(destfile,coment,0);
			}
			f_unlink(destfile);
			f_rename(sourcefile,destfile);
			}
	i++;
    }

    u32 r = 0;
    while(folder[r * 256])
    {
        char* folderpath = (char*)malloc(256);
			if((strlen(&folder[r * 256]) <= 100) & (strlen(&folder[r * 256]) > 0)){			
			strcpy(folderpath, "\0");
			strcat(folderpath, directory);
			strcat(folderpath, "/");
			strcat(folderpath, &folder[r * 256]);
//			deleteall(folderpath, "*","");

        char* folderdest = (char*)malloc(256);
			strcpy(folderdest, "\0");
			strcat(folderdest, destdir);
			strcat(folderdest, "/");
			strcat(folderdest, &folder[r * 256]);
//			deleteall(folderpath, "*","");
			moverall(folderpath, folderdest, filet, coment);
			}
	r++;
    }
}

//move
void f_move (char* file1, char* file2){
	if (sd_file_exists(file1)){
		if (sd_file_exists(file2)){
			f_unlink(file2);			
		}
		f_rename(file1,file2);
	}
}

//copy code
void copyarall(char* directory, char* destdir, char* filet, char* coment){
if (!sd_file_exists(directory)) return;
char* files = listfil(directory, filet, true);
char* folder = listfol(directory, "*", true);
f_mkdir(destdir);
    u32 i = 0;
    while(files[i * 256])
    {
char* sourcefile = (char*)malloc(256);
			if(strlen(&files[i * 256]) <= 100){			
			strcpy(sourcefile, "\0");
			strcat(sourcefile, directory);
			strcat(sourcefile, "/");
			strcat(sourcefile, &files[i * 256]);
			
char* destfile = (char*)malloc(256);
			strcpy(destfile, "\0");
			strcat(destfile, destdir);
			strcat(destfile, "/");
			strcat(destfile, &files[i * 256]);
			if(strlen(coment) > 0){
				printerCU(destfile,coment,0);
			}
			f_unlink(destfile);
			copyfile(sourcefile,destfile);
			}
	i++;
    }

    u32 r = 0;
    while(folder[r * 256])
    {
char* folderpath = (char*)malloc(256);
			if((strlen(&folder[r * 256]) <= 100) & (strlen(&folder[r * 256]) > 0)){			
			strcpy(folderpath, "\0");
			strcat(folderpath, directory);
			strcat(folderpath, "/");
			strcat(folderpath, &folder[r * 256]);
//			deleteall(folderpath, "*","");

char* folderdest = (char*)malloc(256);
			strcpy(folderdest, "\0");
			strcat(folderdest, destdir);
			strcat(folderdest, "/");
			strcat(folderdest, &folder[r * 256]);
//			deleteall(folderpath, "*","");
			copyarall(folderpath, folderdest, filet, coment);
			}
	r++;
    }
}

//folder delete use with care
void deleteall(char* directory, char* filet, char* coment){
if (!sd_file_exists(directory)) return;
char* files = listfil(directory, filet, true);
char* folder = listfol(directory, "*", true);
    u32 i = 0;
    while(files[i * 256])
    {
char* destfile = (char*)malloc(256);
			if(strlen(&files[i * 256]) <= 100){			
			strcpy(destfile, "\0");
			strcat(destfile, directory);
			strcat(destfile, "/");
			strcat(destfile, &files[i * 256]);
			if(strlen(coment) > 0){
				printerCU(destfile,coment,2);
			} else {
				printerCU(destfile,"",2);
			}
				
			f_unlink(destfile);
			}
	i++;
    }

    u32 r = 0;
    while(folder[r * 256])
    {
char* folderpath = (char*)malloc(256);
			if((strlen(&folder[r * 256]) <= 100) & (strlen(&folder[r * 256]) > 0)){			
			strcpy(folderpath, "\0");
			strcat(folderpath, directory);
			strcat(folderpath, "/");
			strcat(folderpath, &folder[r * 256]);
			deleteall(folderpath, filet,coment);
			}
	r++;
    }
f_unlink(directory);
}

bool HasArchBit(const char *directory){
    FRESULT fr;
    FILINFO fno;
    fr = f_stat(directory, &fno);
    if  (fr == FR_OK)
	{
		if (fno.fattrib & AM_ARC)
		{
			return true;
		}
	}	
	return false;
}

void Killflags(char *directory){
	gfx_con_setpos(1, 10);
	printerCU(directory,"",2);
	f_chmod(directory, 0, AM_RDO | AM_ARC);
    if (strstr(directory, "//") != NULL){
        return;
    }
    
    char* folder = listfol(directory, "*", true);
    u32 r = 0;
    while(folder[r * 256])
    {
		char* folderpath = (char*)malloc(256);
			if((strlen(&folder[r * 256]) <= 200) & (strlen(&folder[r * 256]) > 1))
			{		
				strcpy(folderpath, directory);
				strcat(folderpath, "/");
				strcat(folderpath, &folder[r * 256]);
                if (strstr(directory, "//") != NULL) return;
				Killflags(folderpath);
			}
	r++;
    }
}
void fix_emu(){
	if (!(sd_file_exists("sxos/eMMC/00") & sd_file_exists("sxos/eMMC/boot0") & sd_file_exists("sxos/eMMC/boot1")))
		return;

	printerCU("Fix Emu Transfer", "CleanUP...", 0);
	f_mkdir("emuMMC");
	f_mkdir("emuMMC/EF00");
	f_rename("/sxos/eMMC", "/emuMMC/EF00/eMMC");
	f_rename("/Emutendo", "/emuMMC/EF00/Nintendo");

	if (!(sd_file_exists("emuMMC/EF00/eMMC/00") & sd_file_exists("emuMMC/EF00/eMMC/boot0") & sd_file_exists("emuMMC/EF00/eMMC/boot1")))
		return;

	f_unlink("emummc/emummc.bak");
	f_rename("emummc/emummc.ini", "emummc/emummc.bak");
	f_mkdir("emummc");
	FIL fp;
	f_open(&fp, "emummc/emummc.ini", FA_WRITE | FA_CREATE_ALWAYS);
	f_puts("[emummc]\n", &fp);
	f_puts("enabled=1\n", &fp);
	f_puts("path=emuMMC/EF00\n", &fp);
	f_puts("nintendo_path=emuMMC/EF00/Nintendo\n", &fp);
	f_close(&fp);
	sd_save_to_file("", 0, "emuMMC/EF00/file_based");
}

void run_payload(char *path){
    if (sd_file_exists(path)){
        _launch_payload(path, false, true);
    }
}


//ScriptHandler
char *type = "*";
char *clip = "";

void lineHandler(char line[]){
//Simple Instruccion
	/* If line is a comment return */
	if (line[0] == '#')
		return;

	/* If line is a message print it */
	if (line[0] == '[')
	{
		memmove(line, line + 1, strlen(line));
		printerCU(line, "", 0);
		return;
	}
	
	/* If line is a Title print it */
	if (line[0] == '.')
	{
		memmove(line, line + 1, strlen(line));
		printerCU("", line, 0);
		return;
	}

	/* If line is a ArchBit */
	if (line[0] == ':')
	{
		memmove(line, line + 1, strlen(line));
		if (HasArchBit(line))
		{
			printerCU(line, "", 0);
			Killflags(line);
		}
		return;
	}

//Logic instruccion
	/* If line is copy */
	if (line[0] == '<')
	{
		memmove(line, line + 1, strlen(line));
		clip=line;
		return;
	}

	/* If line is paste */
	if (line[0] == '>')
	{
		memmove(line, line + 1, strlen(line));
		copyfile(clip, line);
		return;
	}

	/* If line is rename */
	if (line[0] == '~')
	{
		memmove(line, line + 1, strlen(line));
		f_rename(clip, line);
		return;
	}


	/* If line is a type set type for directories */
	if (line[0] == '*')
	{
		memmove(line, line + 1, strlen(line));
		type = line;
		return;
	}

	/* If line is make folder */
	if (line[0] == '+')
	{
		memmove(line, line + 1, strlen(line));
		if (line[strlen(line) - 1] == '/')
		{
			line[strlen(line) - 1] = 0;
			f_mkdir(line);
			return;
		}
		printerCU(line,"",0);
		sd_save_to_file(clip, strlen(clip), line);
		return;
	}
	
	/* if line is delete */
	if (line[0] == '-')
	{
		memmove(line, line + 1, strlen(line));
		printerCU(line,"",2);

		/* If line is a directory delete it all*/
		if (line[strlen(line) - 1] == '/')
		{
			line[strlen(line) - 1] = 0;
			deleteall(line, type, "");
			//printerCU(line,"",0);
			return;
		}
		/* If line is a file delete */
		if (strlen(line) > 0)
		{
			if (line[0] == '/')
			{
				f_unlink(line);
				return;
			}
		}
	}
}

void clean_up(){
	if (!sd_file_exists("fixer.del")){return;}
    if (btn_read() & BTN_VOL_DOWN){return;}


	FIL delet;
	f_open(&delet, "/StarDust/Main.del", FA_READ);
	FILINFO stats;

	f_stat("/StarDust/Main.del", &stats);
	__off_t size = stats.fsize;

	char buff[size];
	f_read(&delet, buff, size, NULL);

	f_close(&delet);

	strrep(buff, "\r", "");
	char *p = strtok(buff, "\n");
	while (p != NULL)
	{
		lineHandler(p);
		p = strtok(NULL, "\n");
	}
/*
	//not pegascape units
	if (fusesB() > 5)
	{
		printerCU("Remove PegaScape Files", "CleanUP...", 0);
		deleteall("/pegascape", "*", "");
		f_unlink("/switch/fakenews-injector.nro");
	}
*/
	//Fix old Emunand transfer
	fix_emu();

	f_unlink("/fixer.del");
	printerCU("", "", 1); //flush print
}

void Update_SDT(){
	//some test verify payload
	if (sd_file_exists("StarDust/flags/ONE.flag"))
	{
		f_unlink("StarDust/flags/ONE.flag");
		//_launch_payload("payload.bin",false,false);
	}
	//update stardust
	bool cancel_auto_chainloading = btn_read() & BTN_VOL_UP;
	if (sd_file_exists("StarDust_update/fixer.del") & !cancel_auto_chainloading)
	{
		moverall("/StarDust_update", "", "*", "Updating");
		printerCU("Clean Update", "", 0);
		deleteall("/StarDust_update", "*", "Clean Update");
		f_rename("/StarDust_update", "/StarDust_corrupt_update"); //just in case
		//_launch_payload("payload.bin",false,true);
	}
}
void anothermain(){
    //forwarder MAIN SET
    gfx_con.mute = false;
    display_backlight_brightness(h_cfg.backlight, 1000);
    sd_mount();
    
/*    
    if (btn_read_vol() == (BTN_VOL_UP)){
        _launch_payload("/StarDust.bin", false, false);
        _launch_payload("/StarDust_update/StarDust.bin", false, false);
    }
    Update_SDT();
    clean_up();
    if (h_cfg.rcm_patched) {
        f_unlink("/StarDust/sys/minerva.bso");
        f_unlink("/atmosphere/contents/010000000000000D/exefs.nsp");
        f_unlink("/atmosphere/contents/0100000000001013/exefs.nsp");
        //Start Updated Stardust Menu TODO
        _launch_payload("StarDust/payloads/hekate.bin", false, false);
    }
    
    // Mariko Never reach this point
    
    //Ignore Menu, is my way
	if (sd_file_exists("StarDust/flags/UNDERWARE.flag"))
    {
        run_payload("/StarDust/payloads/hekate.bin");
    }
    
    //NOt ignore menu but, Always use this payload, will come handy
    if (!sd_file_exists("StarDust/flags/UNDERTAKE.flag"))
    {
        printerCU("/StarDust.bin --> /payload.bin", "Remove Mariko fix", 0);
        copyfile("/StarDust.bin", "/payload.bin");
	}
    
    //Run menu has normal
    run_payload("/StarDust_update/StarDust.bin");
    run_payload("/StarDust.bin");
    run_payload("/StarDust/payloads/hekate.bin");

    //error handle
    gfx_con.fntsz = 16;
    gfx_printf("Something go Wrong\n\n");
    msleep(5000); // Guard against injection VOL+.
    BootStrapNX();
    
    

*/

    u8 *custom_bg = (u8 *)sd_file_read2("/StarDust/skins/xbox/background.bmp");
    gfx_render_splash(custom_bg);
    
    gfx_render_bmp_arg_file("/StarDust/skins/xbox/Icons/Atmosphere.bmp",100,100,300,300);
    gfx_render_bmp_arg_file("/StarDust/skins/xbox/Icons/Hekate.bmp",500,100,300,300);


        btn_wait();
        msleep(500);  // Guard against force menu VOL-.
        power_set_state(POWER_OFF);

}

/*    
    
    //test
    gfx_con.mute = false;
    display_backlight_brightness(h_cfg.backlight, 1000);
	gfx_con_setpos(10, 5);
    gfx_con.scale = 1;
    gfx_printf("//display_backlight_brightness(h_cfg.backlight, 1000);\n");

    gfx_con.scale = 2;
    gfx_printf("//display_backlight_brightness(h_cfg.backlight, 1000);\n");


    gfx_con.scale = 3;
    gfx_printf("//display_backlight_brightness(h_cfg.backlight, 1000);\n");

    gfx_con.scale = 4;
    gfx_printf("//display_backlight_brightness(h_cfg.backlight, 1000);\n");
    gfx_con.fntsz = 8;
    gfx_printf("//display_backlight_brightness(h_cfg.backlight, 1000);\n");
    msleep(1000); // Guard against injection VOL+.
    btn_wait();
    msleep(500);  // Guard against force menu VOL-.
    power_set_state(POWER_OFF_RESET);
    _launch_payload("/payload.bin", false, true);
*/