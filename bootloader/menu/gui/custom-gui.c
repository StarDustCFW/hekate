/*
 * Copyright (c) 2018 Guillem96
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

#include "custom-gui.h"
#include "gui_menu.h"
#include "../../gfx/gfx.h"
#include "mem/heap.h"
#include "../tools/fs_utils.h"
#include "utils/util.h"
#include "../tools/tools.h"
#include <string.h>


custom_gui_t* custom_gui_load(char *back)
{
    custom_gui_t* custom_gui = (custom_gui_t*)malloc(sizeof(custom_gui_t));
	//custom backgroun for heach menu
	custom_gui->custom_bg = (u8*)sd_4_file_read2(theme(back));

    //custom_gui->title_bmp = (u8*)sd_4_file_read2(CUSTOM_TITLE_PATH);
    return custom_gui;
}

void custom_gui_end(custom_gui_t* cg)
{
    free(cg->custom_bg);
    free(cg->title_bmp);
    free(cg);
}

bool render_custom_background(custom_gui_t* cg)
{
    if (cg->custom_bg == NULL)
        return false;
    
    //gfx_render_bmp_arg_bitmap( cg->custom_bg, 0, 0, 1280, 720);
    gfx_render_splash( cg->custom_bg);
    return true;
}
/*
bool render_custom_title(custom_gui_t* cg)
{  
    if (cg->title_bmp == NULL)
        return false;

    u32 bmp_width = (cg->title_bmp[0x12] | (cg->title_bmp[0x13] << 8) | (cg->title_bmp[0x14] << 16) | (cg->title_bmp[0x15] << 24));
    u32 bmp_height = (cg->title_bmp[0x16] | (cg->title_bmp[0x17] << 8) | (cg->title_bmp[0x18] << 16) | (cg->title_bmp[0x19] << 24));
    gfx_render_bmp_arg_bitmap( cg->title_bmp, 420, 10, bmp_width, bmp_height);
    return true;
}
*/
/*
static void _save_log_to_bmp(char *fname)
{
    //width, height, and bitcount are the key factors:
    s32 width = 720;
    s32 height = 1280;
    u16 bitcount = 32;//<- 24-bit bitmap

    //take padding in to account
    int width_in_bytes = ((width * bitcount + 31) / 32) * 4;

    //total image size in bytes, not including header
    u32 imagesize = width_in_bytes * height;

    //this value is always 40, it's the sizeof(BITMAPINFOHEADER)
    const u32 bi_size = 40;

    //bitmap bits start after headerfile, 
    //this is sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
    const u32 buf_offset_bits = 54; 

    //total file size:
    u32 filesize = 54 + imagesize;

    //number of planes is usually 1
    const u16 bi_planes = 1;






	u32 *fb_ptr = (u32 *)LOG_FB_ADDRESS;

	// Check if there's log written.
	bool log_changed = false;
	for (u32 i = 0; i < 0xCD000; i++)
	{
		if (fb_ptr[i] != 0)
		{
			log_changed = true;
			break;
		}
	}

	if (!log_changed)
		return;

	const u32 file_size = LOG_FB_SZ + 0x36;
	u8 *bitmap = malloc(file_size);

	// Reconstruct FB for bottom-top, landscape bmp. Rotation: 656x1280 -> 1280x656.
	u32 *fb = malloc(LOG_FB_SZ);
	for (int x = 1279; x > - 1; x--)
	{
		for (int y = 655; y > -1; y--)
			fb[y * 1280 + x] = *fb_ptr++;
	}

	manual_system_maintenance(true);

	memcpy(bitmap + 0x36, fb, LOG_FB_SZ);

	typedef struct _bmp_t
	{
		u16 magic;
		u32 size;
		u32 rsvd;
		u32 data_off;
		u32 hdr_size;
		u32 width;
		u32 height;
		u16 planes;
		u16 pxl_bits;
		u32 comp;
		u32 img_size;
		u32 res_h;
		u32 res_v;
		u64 rsvd2;
	} __attribute__((packed)) bmp_t;

	bmp_t *bmp = (bmp_t *)bitmap;

	bmp->magic    = 0x4D42;
	bmp->size     = file_size;
	bmp->rsvd     = 0;
	bmp->data_off = 0x36;
	bmp->hdr_size = 40;
	bmp->width    = 1280;
	bmp->height   = 656;
	bmp->planes   = 1;
	bmp->pxl_bits = 32;
	bmp->comp     = 0;
	bmp->img_size = LOG_FB_SZ;
	bmp->res_h    = 2834;
	bmp->res_v    = 2834;
	bmp->rsvd2    = 0;

	char path[0x80];
	strcpy(path, "StarDust");
	s_printf(path + strlen(path), "/nyx%s_log.bmp", fname);
	sd_save_to_file(bitmap, file_size, path);

	free(bitmap);
	free(fb);
}

*/

int screenshot(void* params)
{
SDStrap();
    //width, height, and bitcount are the key factors:
    s32 width = 720;
    s32 height = 1280;
    u16 bitcount = 32;//<- 24-bit bitmap

    //take padding in to account
    int width_in_bytes = ((width * bitcount + 31) / 32) * 4;

    //total image size in bytes, not including header
    u32 imagesize = width_in_bytes * height;

    //this value is always 40, it's the sizeof(BITMAPINFOHEADER)
    const u32 bi_size = 40;

    //bitmap bits start after headerfile, 
    //this is sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER)
    const u32 buf_offset_bits = 54; 

    //total file size:
    u32 filesize = 54 + imagesize;

    //number of planes is usually 1
    const u16 bi_planes = 1;

    //create header:
    //copy to buffer instead of BITMAPFILEHEADER and BITMAPINFOHEADER
    //to avoid problems with structure packing
    unsigned char header[54] = { 0 };
    memcpy(header, "BM", 2);
    memcpy(header + 2 , &filesize, 4);
    memcpy(header + 10, &buf_offset_bits, 4);
    memcpy(header + 14, &bi_size, 4);
    memcpy(header + 18, &height, 4);
    memcpy(header + 22, &width, 4);
    memcpy(header + 26, &bi_planes, 2);
    memcpy(header + 28, &bitcount, 2);
    memcpy(header + 34, &imagesize, 4);
	

	char namef[512];
	strcpy(namef, "StarDust/screenshot.bmp");
	char tmp[512];
	strcpy(tmp,namef);
	for (int i=1;i < 10;i++){
		if (sd_file_exists(tmp)){
			char charValue=i+'0';
			char tmp2[2];
			tmp2[0]=charValue;
			tmp2[1]='\0';
			
			strcpy(tmp, "\0");
			strcpy(tmp, "StarDust/screenshot_");
			strcat(tmp, tmp2);
			strcat(tmp, ".bmp");
			if (!sd_file_exists(tmp)){
				strcpy(namef, tmp);
			}
		} else break;
	}
    u8* buff = (u8*)malloc(imagesize + 54);
    memcpy(buff, header, 54);
    
    u32* buffer = gfx_ctxt.fb + gfx_ctxt.width * gfx_ctxt.stride * 4;;
    u32* center = gfx_ctxt.fb;
    
    int x, y;
    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            /*
            // Calcular las coordenadas del píxel rotado en dirección horaria y volteado horizontalmente
            int new_x = y;
            int new_y = width - x - 1;
            // Obtener el índice del píxel en el buffer rotado
            int index_rotado = new_y * height + new_x;
            int index_original = y * width + x;
            // Copiar el píxel al buffer rotado
            
            // Calcular las coordenadas del píxel rotado en sentido horario
            int new_x = height - y - 1;
            int new_y = x;
            // Obtener el índice del píxel en el buffer rotado
            int index_rotado = new_y * height + new_x;
            int index_original = y * width + x;
            // Copiar el píxel al buffer rotado            
            */
            // Calcular las coordenadas del píxel rotado en sentido antihorario y volteado horizontalmente
            int new_x = height - y - 1;
            int new_y = width - x - 1;
            // Obtener el índice del píxel en el buffer rotado
            int index_rotado = new_y * height + new_x;
            int index_original = y * width + x;
            // Copiar el píxel al buffer rotado            
            buffer[index_rotado] = center[index_original];
        }
    }

    memcpy(buff + 54, buffer, imagesize);

    //gfx_render_bmp_arg_bitmap(buff, 1, 5, 1280, 720);

    sd_save_2_file(buff, imagesize + 54, namef);
    free(buff);

    gfx_con.scale = 2;
    gfx_con_setpos( 0, 605);
    gfx_printf( " Screenshot saved!\n Find it at %s",namef);
    return 0;
}
