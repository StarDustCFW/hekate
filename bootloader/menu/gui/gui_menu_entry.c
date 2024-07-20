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

#include "gui_menu_entry.h"

#include "mem/heap.h"

#include "../../gfx/gfx.h"

#include "../tools/touch2.h"
#include "../tools/fs_utils.h"

#include <string.h>

#define DEFAULT_LOGO "StarDust/icons/default.bmp"

/* Creates a menu entry */
gui_menu_entry_t *gui_create_menu_entry(const char *text,
                                        u8 *bitmap,
                                        u32 x, u32 y,
                                        u32 width, u32 height,
                                        int (*handler)(void *), void *param)
{
    gui_menu_entry_t *menu_entry = (gui_menu_entry_t *)malloc(sizeof(gui_menu_entry_t));
    strcpy(menu_entry->text, text);

    if (bitmap != NULL)
    {
        /* When not using the default icon set the text empty */
        /* User knows which icon he uses for each payload */
        menu_entry->bitmap = bitmap;
        strcpy(menu_entry->text, ""); // If not default icon, text is not needed on touch input
    }
    else
        menu_entry->bitmap = sd_4_file_read2(DEFAULT_LOGO);
    menu_entry->bit = true;
    menu_entry->type = 0;
    menu_entry->x = x;
    menu_entry->y = y;
    menu_entry->width = width;
    menu_entry->height = height;
    menu_entry->handler = handler;
    menu_entry->param = param;
    return menu_entry;
}
gui_menu_entry_t *gui_create_menu_entry_switch(const char *text,
                                        u8 *bitmap,
                                        u8 *bitmap2,
                                        bool bit,
                                        u32 x, u32 y,
                                        u32 width, u32 height,
                                        int (*handler)(void *), void *param,int (*handler2)(void *), void *param2)
{
    gui_menu_entry_t *menu_entry = (gui_menu_entry_t *)malloc(sizeof(gui_menu_entry_t));
    strcpy(menu_entry->text, text);

    menu_entry->bitmap = bitmap;
    menu_entry->bitmap2 = bitmap2;
    menu_entry->bit = bit;
    menu_entry->type = 1;
    menu_entry->x = x;
    menu_entry->y = y;
    menu_entry->width = width;
    menu_entry->height = height;
    menu_entry->handler = handler;
    menu_entry->param = param;
    menu_entry->handler2 = handler2;
    menu_entry->param2 = param2;
    return menu_entry;
}

gui_menu_entry_t *gui_create_menu_entry_no_bitmap(const char *text,
                                                  u32 x, u32 y,
                                                  u32 width, u32 height,
                                                  int (*handler)(void *), void *param)
{
    gui_menu_entry_t *menu_entry = (gui_menu_entry_t *)malloc(sizeof(gui_menu_entry_t));
    strcpy(menu_entry->text, text);
    menu_entry->bitmap = NULL;
    menu_entry->x = x;
    menu_entry->y = y;
    menu_entry->width = width;
    menu_entry->height = height;
    menu_entry->handler = handler;
    menu_entry->param = param;
    return menu_entry;
}

/* Get text width */
static u32 get_text_width(char *text)
{
    u32 lenght = strlen(text);
    //return lenght * gfx_con.scale * (u32)CHAR_WIDTH;
    return lenght * gfx_con.fntsz;
}

static void render_text_centered(gui_menu_entry_t *entry, char *text)
{
    s32 x_set = entry->x;
    u32 y_set = entry->y;

   // gfx_con.scale = 2;
    if(entry->bitmap != NULL){
        x_set = x_set + (entry->width /2) - (get_text_width(text) / 2);
        y_set = y_set + (entry->height /2) - (gfx_con.fntsz/2);
    } else {
        x_set = x_set - (get_text_width(text) / 2);
    }
    // Set text below the logo and centered  (entry->width - get_text_width(text))/ 2
    // s32 x_set = entry->bitmap != NULL ? 0 : get_text_width(text) / 2;
    // u32 y_set = entry->bitmap != NULL ? entry->height /2 - (gfx_con.fntsz/2): 0;

    //gfx_con.scale = 2;
    gfx_con_setpos(x_set, y_set);

    gfx_printf( "%s", entry->text);
/*
    gfx_con_setpos(0, 115);
	gfx_con_setcol( 0xFFF9F9F9, 0xFFFFFFFF, 0xFF191414);

    //gfx_printf( "X  %d\n", entry->x);
    gfx_printf( "Y  %d\n", entry->y);
    gfx_printf( "oX %d   \n", x_set);
    gfx_printf( "oY %d   \n", y_set);

    gfx_printf( "\n%d\n", gfx_con.fntsz);
    gfx_con_setcol( 0xFFF9F9F9, 0, 0xFF191414);

    msleep(200);
*/    

}

/* Renders a gfx menu entry */
void gui_menu_render_entry(gui_menu_entry_t* entry)
{
    if (entry->bit){
        gfx_render_bmp_arg_bitmap( entry->bitmap,
                                    entry->x, entry->y,
                                    entry->width, entry->height);
    }else{
        gfx_render_bmp_arg_bitmap( entry->bitmap2,
                                    entry->x, entry->y,
                                    entry->width, entry->height);
    }


    if (strlen(entry->text) > 1)
        render_text_centered(entry, entry->text);
}

void gui_menu_entry_destroy(gui_menu_entry_t *entry)
{
    free(entry->bitmap);
    free(entry->bitmap2);
    free(entry->text);
    free(entry->param);
    free(entry);
}
