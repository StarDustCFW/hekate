/*
 * Copyright (c) 2018 naehrwert
 * Copyright (C) 2018 CTCaer
 * Copyright (C) 2018 Guillem96
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
#include "utils/dirlist.h"
#include "../tools/fs_utils.h"
#include "utils/util.h"
#include "display/di.h"
#include "mem/heap.h"
#include "../../gfx/gfx.h"
#include <string.h>
#include <stdlib.h>
#include "../tools/tools.h"

/*
typedef enum {
	FR_OK = 0,				// (0) Succeeded 
	FR_DISK_ERR,			// (1) A hard error occurred in the low level disk I/O layer 
	FR_INT_ERR,				// (2) Assertion failed 
	FR_NOT_READY,			// (3) The physical drive cannot work 
	FR_NO_FILE,				// (4) Could not find the file 
	FR_NO_PATH,				// (5) Could not find the path 
	FR_INVALID_NAME,		// (6) The path name format is invalid 
	FR_DENIED,				// (7) Access denied due to prohibited access or directory full 
	FR_EXIST,				// (8) Access denied due to prohibited access 
	FR_INVALID_OBJECT,		// (9) The file/directory object is invalid 
	FR_WRITE_PROTECTED,		// (10) The physical drive is write protected 
	FR_INVALID_DRIVE,		// (11) The logical drive number is invalid 
	FR_NOT_ENABLED,			// (12) The volume has no work area 
	FR_NO_FILESYSTEM,		// (13) There is no valid FAT volume 
	FR_MKFS_ABORTED,		// (14) The f_mkfs() aborted due to any problem 
	FR_TIMEOUT,				// (15) Could not get a grant to access the volume within defined period 
	FR_LOCKED,				// (16) The operation is rejected according to the file sharing policy 
	FR_NOT_ENOUGH_CORE,		// (17) LFN working buffer could not be allocated 
	FR_TOO_MANY_OPEN_FILES,	// (18) Number of open files > FF_FS_LOCK 
#if FF_FASTFS
	FR_INVALID_PARAMETER,	// (19) Given parameter is invalid 
	FR_CLTBL_NO_INIT	    // (20) The cluster table for fast seek/read/write was not created 
#else
	FR_INVALID_PARAMETER	// (19) Given parameter is invalid 
#endif
} FRESULT;


*/

bool delete_file(const char* filename) {
    FRESULT fr = f_unlink(filename);
    if (fr == FR_OK) {
        // Eliminación exitosa
        return true;
    } else {
        // Error al eliminar el archivo
        gfx_printf("Error al eliminar el archivo '%s':# %d\r\n", filename,fr);
        //msleep(5000);
        return false;
    }
}
void *sd_4_file_read2(const char *path)
{
    FIL fp;
    if (f_open(&fp, path, FA_READ) != FR_OK)
        return NULL;  // Error al abrir el archivo, retorna NULL

    u32 size = f_size(&fp);
    void *buf = malloc(size + 1);  // Tamaño del buffer más un byte para el carácter nulo
    if (!buf)
    {
        f_close(&fp);  // Cierra el archivo antes de salir
        return NULL;   // Error de asignación de memoria, retorna NULL
    }

    u8 *ptr = buf;
    while (size > 0)
    {
        u32 rsize = MIN(size, 512 * 512);
        UINT br;
        if (f_read(&fp, ptr, rsize, &br) != FR_OK || br != rsize)
        {
            free(buf);     // Libera la memoria antes de salir
            f_close(&fp);  // Cierra el archivo antes de salir
            return NULL;   // Error al leer o tamaño incorrecto, retorna NULL
        }

        ptr += rsize;
        size -= rsize;
    }

    *ptr = '\0';  // Agrega el carácter nulo al final del buffer
    f_close(&fp); // Cierra el archivo después de leer

    return buf;
}

char *read_file_string(const char *path)
{
    FIL file;
    FILINFO stats;
    FRESULT res;
    unsigned int size;

    res = f_open(&file, path, FA_READ);
    if (res != FR_OK)
    {
        // Manejo de error al abrir el archivo
        // Puedes imprimir un mensaje de error o registrar el problema
        return NULL;
    }

    res = f_stat(path, &stats);
    if (res != FR_OK)
    {
        // Manejo de error al obtener información del archivo
        f_close(&file);  // Cerrar el archivo antes de salir
        return NULL;
    }

    size = stats.fsize;
    char *buff = malloc(size + 1);  // Reserva memoria para el contenido más el carácter nulo
    if (!buff)
    {
        // Manejo de error al asignar memoria
        f_close(&file);  // Cerrar el archivo antes de salir
        return NULL;
    }

    res = f_read(&file, buff, size, &size);
    if (res != FR_OK)
    {
        // Manejo de error al leer el archivo
        free(buff);       // Liberar memoria antes de salir
        f_close(&file);   // Cerrar el archivo antes de salir
        return NULL;
    }

    buff[size] = '\0';  // Agrega el carácter nulo al final de la cadena
    f_close(&file);     // Cierra el archivo después de leer

    return buff;
}

int sd_save_2_file(void *buf, u32 size, const char *filename)
{
    FIL fp;
    FRESULT res;

    // Intenta abrir el archivo en modo de creación siempre y escritura
    res = f_open(&fp, filename, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        gfx_printf("%kError (%d) creating file\n%s.\n%k\n", 0xFFFFDD00, res, filename, 0xFFCCCCCC);
        return 1;
    }

    // Escribe el contenido en el archivo
    res = f_write(&fp, buf, size, NULL);
    if (res != FR_OK)
    {
        gfx_printf("%kError (%d) writing to file\n%s.\n%k\n", 0xFFFFDD00, res, filename, 0xFFCCCCCC);
        f_close(&fp);  // Cerrar el archivo antes de salir
        return 2;
    }

    // Cierra el archivo después de escribir
    res = f_close(&fp);
    if (res != FR_OK)
    {
        gfx_printf("%kError (%d) closing file\n%s.\n%k\n", 0xFFFFDD00, res, filename, 0xFFCCCCCC);
        return 3;
    }

    return 0;  // Todo salió bien
}

bool sd_file_exists(const char* filename)
{
    if (filename == NULL) {
        // Manejar caso de nombre de archivo nulo
        return false;
    }    FRESULT fr;
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

bool sd_file_size(char *path)
{
	FIL fp;
	if (f_open(&fp, path, FA_READ) != FR_OK)
		return 0;

	u32 size = f_size(&fp);
	f_close(&fp);
	return size;
}

void copyfile(const char* source, const char* target)
{
    FIL fp_source, fp_target;
    FRESULT res;

    // Abrir el archivo de origen para lectura
    res = f_open(&fp_source, source, FA_READ);
    if (res != FR_OK)
    {
        gfx_printf("File %s missing\n", source);
        gfx_swap_buffer();
        return;  // Salir si no se puede abrir el archivo de origen
    }

    // Abrir el archivo de destino para escritura (crear si no existe)
    res = f_open(&fp_target, target, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK)
    {
        gfx_printf("Error creating file %s\n", target);
        gfx_swap_buffer();
        f_close(&fp_source);  // Cerrar archivo de origen antes de salir
        return;  // Salir si no se puede crear el archivo de destino
    }

    // Leer y escribir datos del archivo de origen al archivo de destino
    static uint8_t buf[512];  // Buffer temporal para la lectura/escritura
    UINT bytes_read, bytes_written;
    while (f_read(&fp_source, buf, sizeof(buf), &bytes_read) == FR_OK && bytes_read > 0)
    {
        f_write(&fp_target, buf, bytes_read, &bytes_written);
        if (bytes_written != bytes_read)
        {
            gfx_printf("Error writing to file %s\n", target);
            gfx_swap_buffer();
            break;  // Salir si hay un error al escribir
        }
    }

    // Cerrar los archivos después de la operación de copia
    f_close(&fp_source);
    f_close(&fp_target);
}


void copyfileparam(char* param, char* source, char* target)
{
	char* path = (char*)malloc(256);
    strcpy(path, param);
    strcat(path, "/");
    strcat(path, source);
/*		
		gfx_con.scale = 2;
        gfx_con_setpos( 15, 50);
		gfx_printf( "--------------\n",path);
		gfx_printf( "copy %s %s\n",path ,target);
	    gfx_swap_buffer();
*/		
    FIL fp;
    if (f_open(&fp, path, FA_READ) != FR_OK)
    {
		gfx_printf( "file %s mising\n",path);
	    gfx_swap_buffer();
		msleep(3000);
    }else{
        u32 size = f_size(&fp);
        f_close(&fp);
        sd_save_2_file(sd_4_file_read2(path),size,target);
	}
}

void copy_folder(char* sourse_folder, char* dest_folder)
{
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
            gfx_con_setpos( 10, 90);
            gfx_printf( "\ncopy %s to %s\n",source_file,dest_file);
            gfx_swap_buffer();
            copyfile(source_file,dest_file);//action
        }
        i++;
    }
}

//move code
void moverall(char* directory, char* destdir, char* filet, char* coment)
{
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
void f_move (char* file1, char* file2)
{
	if (sd_file_exists(file1)){
		if (sd_file_exists(file2)){
			f_unlink(file2);			
		}
		f_rename(file1,file2);
	}
}

//copy code
void copyarall(char* directory, char* destdir, char* filet, char* coment)
{
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
void deleteall(char* directory, char* filet, char* coment)
{
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

bool HasArchBit(const char *directory)
{
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

void Killflags(char *directory)
{
    FRESULT res;
    DIR dir;
    static FILINFO fno;
    char *folder = NULL;

    res = f_opendir(&dir, directory);
    if (res != FR_OK)
        return;

    res = f_readdir(&dir, &fno);
    if (res != FR_OK || fno.fname[0] == 0)
        goto close_dir;

    gfx_con_setpos(1, 10);
    printerCU(directory, "", 2);
    f_chmod(directory, 0, AM_ARC);
    if (strstr(directory, "//") != NULL)
        goto close_dir;

    folder = listfol(directory, "*", true);
    if (!folder)
        goto close_dir;

    u32 r = 0;
    while (folder[r * 256] != 0)
    {
        char *folderpath = (char *)malloc(256);
        if (!folderpath)
            break;  // Error de asignación de memoria

        if ((strlen(&folder[r * 256]) <= 200) && (strlen(&folder[r * 256]) > 1))
        {
            strcpy(folderpath, directory);
            strcat(folderpath, "/");
            strcat(folderpath, &folder[r * 256]);
            Killflags(folderpath);
        }
        free(folderpath); // Liberar memoria dentro del bucle
        r++;
    }

close_dir:
    free(folder);  // Liberar memoria de folder
    f_closedir(&dir);
}


char *listfol(const char *directory, const char *pattern, bool includeHiddenFiles)
{
	u8 max_entries = 61;

	int res = 0;
	int i = 0, j = 0, k = 0;
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

char *listfil(const char *directory, const char *pattern, bool includeHiddenFiles)
{
	u8 max_entries = 61;

	int res = 0;
	int i = 0, j = 0, k = 0;
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
