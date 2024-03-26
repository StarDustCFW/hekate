/*
 * Copyright (c) 2018 langerhans
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
#include "utils/util.h"
#include "fs_utils.h"
#include "utils/btn.h"
#include "../gui/custom-gui.h"
#include "../tools/touch2.h"
#include "../../gfx/gfx.h"
#include "tools.h"

extern gfx_ctxt_t gfx_ctxt;

touch_event last_event;
/*
static int touch_command(u8 cmd)
{
	int err = i2c_send_byte(I2C_3, 0x49, cmd, 0);
	if (err < 0)
		return err;
		
	// TODO: Check for completion in event loop
	msleep(10);
	return 0;
}

static void touch_process_contact_event(touch_event *event)
{
	event->x = (event->raw[3] << 4) | ((event->raw[3] & 0xf0) >> 1);
	event->y = (event->raw[2] << 4) | ((event->raw[3] & 0x0f));
}

static void touch_parse_event(touch_event *event) 
{
	event->type = event->raw[0];

	switch (event->type) 
	{
	case STMFTS_EV_NO_EVENT:
	case STMFTS_EV_CONTROLLER_READY:
	case STMFTS_EV_SLEEP_OUT_CONTROLLER_READY:
	case STMFTS_EV_STATUS:
		return;

	case STMFTS_EV_MULTI_TOUCH_MOTION:
	case STMFTS_EV_MULTI_TOUCH_ENTER:
	case STMFTS_EV_MULTI_TOUCH_LEAVE:
		touch_process_contact_event(event);
		break;
	}
}

static void touch_poll(touch_event *event)
{
    i2c_recv_buf_small(event->raw, 4, I2C_3, 0x49, STMFTS_LATEST_EVENT);
    touch_parse_event(event);
}
*/
touch_event touch_wait()
{
	touch_event event;
    static u32 maar =0;
	do 
	{
		//some functions to buttons on touch wait
		if (btn_read() & BTN_VOL_DOWN) power_set_state(POWER_OFF);
		
		if (btn_read() & BTN_POWER)	BootStrapNX();
	
		touch_poll(&event);
		if (event.type == STMFTS_EV_MULTI_TOUCH_ENTER || event.type == STMFTS_EV_MULTI_TOUCH_MOTION){
			maar=1;
			if (event.touch)//(event.y > 20 & event.x > 100)
			{
                /*
                if (event.y < 1280 & event.x < 1280){
                    gfx_con_setcol( 0xFFCCCCCC, 0xFFCCCCCC, 0xFF191414);
                    gfx_con_setpos(225, 225);
                    gfx_printf( "X:%d--Y:%d",event.y, event.x);
                    //draw pointier o enter
                    gfx_con.scale = 5;
                    gfx_con_setpos( event.x-25,event.y-25);
                    gfx_printf( "X");
                }
                */
			}
			
		} else if(maar==1) {
			//if not touching screen, then leave just once per touch
			maar=0;
			event.type = STMFTS_EV_MULTI_TOUCH_LEAVE;
                
            //gfx_swap_buffer();
		} else {
			//this evoid the repeated event
			event.type = STMFTS_EV_NO_EVENT;
		}

		//if (event.type == STMFTS_EV_MULTI_TOUCH_LEAVE || event.type == STMFTS_EV_MULTI_TOUCH_ENTER ||  event.type == STMFTS_EV_MULTI_TOUCH_MOTION ) break;

	} while(event.type != STMFTS_EV_MULTI_TOUCH_LEAVE);
    
	return event;
}
/*
int touch_power_on() 
{
	int err;

	 // The datasheet does not specify the power on time, but considering
	//  that the reset time is < 10ms, I sleep 20ms to be sure

	msleep(20);
	
	err = touch_command(STMFTS_SYSTEM_RESET);
	if (err < 0)
		return err;

	err = touch_command(STMFTS_SLEEP_OUT);
	if (err < 0)
		return err;

	// err = touch_command(STMFTS_MS_CX_TUNING);
	// if (err < 0)
	// 	return err;

	// err = touch_command(STMFTS_SS_CX_TUNING);
	// if (err < 0)
	// 	return err;

	err = touch_command(STMFTS_FULL_FORCE_CALIBRATION);
	if (err < 0)
		return err;

	err = touch_command(STMFTS_MS_MT_SENSE_ON);
	if (err < 0)
		return err;

	return 1;
}
*/
bool is_rect_touched(touch_event* event, u32 x, u32 y, u32 width, u32 height)
{
    if (event == NULL)
        return false;
        
    u32 event_x = event->x;
    u32 event_y = event->y;
    //if(true) return false;
    return event_x > x 
            && event_y > y
            && event_x < x + width
            && event_y < y + height;
}
