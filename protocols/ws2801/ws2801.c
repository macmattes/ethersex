/*
 * Copyright (c) 2009 by Stefan Krupop <mail@stefankrupop.de>
 * Copyright (c) 2009 by Dirk Pannenbecker <dp@sd-gp.de>
 * Copyright (c) 2011-2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 *
 * ----------------------------------------------------------------------------------|
 * |                                                                                 |
 * | Original Project : http://www.dmxcontrol.de/wiki/Art-Net-Node_f%C3%BCr_25_Euro  |
 * | Copied Version   : 17.01.2009                                                   |
 * |                                                                                 |
 * ----------------------------------------------------------------------------------|
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/bool.h"
#include "core/bit-macros.h"
#include "protocols/ws2801/ws2801.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "services/dmx-storage/dmx_storage.h"
#ifdef WS2801_SUPPORT

/* ----------------------------------------------------------------------------
 *global variables
 */

uint8_t ws2801_subNet = SUBNET_DEFAULT;
uint8_t ws2801_artnet_universe = CONF_WS2801_UNIVERSE;
uint16_t ws2801_colortemp = 6500;
uint8_t ws2801_status = RC_POWER_OK;
uint16_t ws2801_port = CONF_WS2801_PORT;

volatile unsigned char  ws2801_dmxUniverse[512];
volatile uint16_t ws2801_artnetChannels;
uint8_t ws2801_pixels = CONF_WS2801_PIXELS;
uint16_t ws2801_channels = CONF_WS2801_PIXELS*3;

const char ws2801_ID[8] PROGMEM = "Art-Net";
const uint16_t pwmtable_8C[16] PROGMEM =
{
    0, 2, 4, 6, 10, 14, 20, 28, 35, 45, 59, 76, 97, 128, 181, 255 
};

/*const uint16_t pwmtable_8C[16] PROGMEM =
{
    0, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64, 90, 128, 181, 255
};*/

volatile uint32_t ramp_up_timer = 0;
volatile uint32_t ramp_down_timer = 0;
volatile uint32_t onfor_timer = 0;
volatile uint16_t ws2801_ramp_up_timer = 1;
volatile uint16_t ws2801_ramp_down_timer = 1;
volatile uint16_t ws2801_timer = 0;
volatile uint16_t ws2801_dim_steps = 0;
volatile int ws2801_state = 0;
volatile int ws2801_artnet_state = 0;
volatile int ws2801_dim_state = 15;
volatile int ws2801_slow_dim_state = 0;
volatile int ws2801_dim_direction = 0;
volatile int periodic_state = WS2801_STATE_IDLE;

uint16_t milli = 50;

uint8_t ws2801_r, ws2801_g, ws2801_b;

void ws2801_writebyte(unsigned char);
void ws2801_showpixel(void);

void hsv_to_rgb(double h, double s, double v, double *R, double *G, double *B);
void rgb_to_hsv(double r, double g, double b, double *H, double *S, double *V);

/* ----------------------------------------------------------------------------
 * initialization of network settings
 */
void
ws2801_netInit(void)
{
  ws2801_net_init();
}

/* ----------------------------------------------------------------------------
 * initialization of Art-Net
 */
void
ws2801_init(void)
{
  WS2801_DEBUG("Init\n");

  /* net_init */
  ws2801_netInit();

  /* enable ws2801 */
  PIN_CLEAR(WS2801_CLOCK);
  PIN_CLEAR(WS2801_DATA);

  /* set startcolor */
  ws2801_colortemp_set(ws2801_colortemp);
  ws2801_storage_write();
  ws2801_clear();

#ifdef WS2801_ARTNET_STARTUP
  /* enable artnet */
  ws2801_artnet_state = 1;
#endif

  WS2801_DEBUG("init complete\n");
  return;
}

uint8_t new=0;

void
ws2801_out(void)
{
  if (new == 1) {
    ws2801_storage_show();  //storage ausgeben
    new = 0;
  }
}

/* ----------------------------------------------------------------------------
 * receive Art-Net packet
 */
void
ws2801_get(void)
{
  if (new == 0) {
  struct ws2801_header *header;

  header = (struct ws2801_header *) uip_appdata;

  /* check the id */
  if (strncmp_P((char *) header->id, ws2801_ID, 8))
  {
    WS2801_DEBUG("Wrong ArtNet header, discarded\r\n");
    ws2801_status = RC_PARSE_FAIL;
    return;
  }
  switch (header->opcode)
  {
    case OP_POLL:;
    case OP_POLLREPLY:;
    case OP_OUTPUT:;
      struct ws2801_dmx *dmx;

      WS2801_DEBUG("Received ws2801 output packet!\r\n");
      dmx = (struct ws2801_dmx *) uip_appdata;
     if (ws2801_artnet_state == 1) 
     {
      if (dmx->universe == ((ws2801_subNet << 4) | ws2801_artnet_universe))
      {
	  ws2801_artnetChannels = (dmx->lengthHi << 8) | dmx->length;
          memcpy((unsigned char*)&ws2801_dmxUniverse[0], &(dmx->dataStart), ws2801_artnetChannels);
          new = 1;
	  //ws2801_storage_show();  //storage ausgeben
      }
    }
      break;
    case OP_ADDRESS:;
    case OP_IPPROG:;
      break;
    default:
      WS2801_DEBUG("Received an invalid ws2801 packet!\r\n");
      break;
  }
 } //new empty?
}

void ws2801_color_set(uint8_t r, uint8_t g, uint8_t b)
{
	ws2801_r = r;
	ws2801_g = g;
	ws2801_b = b;
}

void ws2801_colortemp_set(uint16_t k)
{
    ws2801_colortemp = k;
    float Temperature,Red,Green,Blue;
    Temperature = k / 100;
    
    //Calculate Red:

    if (Temperature <= 66){
        Red = 255;
    } else {
        Red = Temperature - 60;
        Red = 329.698727446 * pow(Red,-0.1332047592);
        if (Red < 0) {
		Red = 0;
	}
        if (Red > 255) {
		Red = 255;
	}
    }
    
    //Calculate Green:

    if (Temperature <= 66){
        Green = Temperature;
        Green = 99.4708025861 * log(Green) - 161.1195681661;
        if (Green < 0){
		Green = 0;
	}
        if (Green > 255){
		Green = 255;
    	}
    } else {
        Green = Temperature - 60;
        Green = 288.1221695283 * pow(Green,-0.0755148492);
        if (Green < 0){
		Green = 0;
	}
        if (Green > 255) {
		Green = 255;
	}
    }
    
    //Calculate Blue:

    if (Temperature >= 66){
        Blue = 255;
    } else {
        if (Temperature <= 19) {
            Blue = 0;
        } else {
            Blue = Temperature - 10;
            Blue = 138.5177312231 * log(Blue) - 305.0447927307;
            if (Blue < 0) {
		 Blue = 0;
	    }
            if (Blue > 255) {
		Blue = 255;
	    }
        }

    }
    ws2801_color_set(Red,Green,Blue);
}

void ws2801_clear(void)
{
  if (ws2801_artnet_state == 0) {
	uint16_t dmxch;
	for(dmxch = 0; dmxch < ws2801_channels; dmxch++)
	{
		ws2801_writebyte(0);
	}
    
    	if (dmxch == ws2801_channels) {
    		ws2801_showpixel();
    	}
  }
}

void ws2801_storage_write()
{ 
	if (ws2801_artnet_state == 0) {
		if (ws2801_dim_state >= 0) { 
			double r,g,b,H,S,V;
			rgb_to_hsv(ws2801_r,ws2801_g,ws2801_b,&H,&S,&V);
			//hsv_to_rgb(H,S,ws2801_dim_state,&r,&g,&b);
			hsv_to_rgb(H,S,pgm_read_word (& pwmtable_8C[ws2801_dim_state]),&r,&g,&b);

			uint16_t dmxpx;
			for(dmxpx = 0; dmxpx < ws2801_pixels; dmxpx++)
			{
				ws2801_dmxUniverse[(dmxpx*3)+0] = (uint8_t) r;
				ws2801_dmxUniverse[(dmxpx*3)+1] = (uint8_t) g;
				ws2801_dmxUniverse[(dmxpx*3)+2] = (uint8_t) b;
			}
		}
	}
}


double r,g,b,H,S,V,r_n,g_n,b_n;
void ws2801_storage_write_slowdim()
{ 
	if (ws2801_artnet_state == 0) {
		if (ws2801_slow_dim_state > 0) { 
			rgb_to_hsv(ws2801_r,ws2801_g,ws2801_b,&H,&S,&V);
			//hsv_to_rgb(H,S,ws2801_slow_dim_state,&r,&g,&b);
			hsv_to_rgb(H,S,pgm_read_word (& pwmtable_8C[ws2801_slow_dim_state]),&r_n,&g_n,&b_n);
			r=((ws2801_dim_steps-1)*r+r_n)/ws2801_dim_steps;
			g=((ws2801_dim_steps-1)*g+g_n)/ws2801_dim_steps;
			b=((ws2801_dim_steps-1)*b+b_n)/ws2801_dim_steps;
			uint16_t dmxpx;
			for(dmxpx = 0; dmxpx < ws2801_pixels; dmxpx++)
			{
				ws2801_dmxUniverse[(dmxpx*3)+0] = (uint8_t) r;
				ws2801_dmxUniverse[(dmxpx*3)+1] = (uint8_t) g;
				ws2801_dmxUniverse[(dmxpx*3)+2] = (uint8_t) b;
			}
		} else {
			uint16_t dmxpx;
			for(dmxpx = 0; dmxpx < ws2801_pixels; dmxpx++)
			{
				ws2801_dmxUniverse[(dmxpx*3)+0] = 0;
				ws2801_dmxUniverse[(dmxpx*3)+1] = 0;
				ws2801_dmxUniverse[(dmxpx*3)+2] = 0;
			}
		}
		
	}
}

// HSV-RGB Conversion-------------------------------------------------------------
void
hsv_to_rgb(double h, double s, double v, double *R, double *G, double *B)
{
    int segment;
    double *rgb[3], major, minor, middle, frac;

    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;

    while (h < 0)
	h++;
    while (h >= 1)
	h--;

    segment = (int)(h*6);

    frac = (6*h)-segment;
    if (segment % 2)
	frac = 1 - frac;

    major = v;
    minor = (1-s)*v;
    middle = frac * major + (1-frac) * minor;

    *rgb[(segment+1)/2 % 3] = major;
    *rgb[(segment+4)/2 % 3] = minor;
    *rgb[(7-segment) % 3] = middle;
}

#define swap(a,b) (temp = (a), (a) = (b), (b) = temp)
#define rshift(a,b,c) (temp = (c), (c) = (b), (b) = (a), (a) = temp)
void
rgb_to_hsv(double r, double g, double b, double *H, double *S, double *V)
{
    int segment;
    double *rgb[3], *temp, frac;

    rgb[0] = &r;
    rgb[1] = &g;
    rgb[2] = &b;

    /* sort rgb into increasing order */

    if (*rgb[0] > *rgb[1]) {
	swap(rgb[0], rgb[1]);
    }
    if (*rgb[1] > *rgb[2]) {
	if (*rgb[0] > *rgb[2]) {
	    rshift(rgb[0], rgb[1], rgb[2]);
	}else{
	    swap(rgb[1], rgb[2]);
	}
    }
    if (*V = *rgb[2]) {
	if (*S = 1 - *rgb[0] / *rgb[2]) {
	    /* segment = 3*(b>g) + 2 * (rgb[1]==&b) + (rgb[1]==&r); */
	    segment = 3 * (rgb[0]==&g||rgb[2]==&b)
		    + 2 * (rgb[1]==&b) + (rgb[1]==&r);
	    frac = (*rgb[1] - *rgb[0]) / (*rgb[2] - *rgb[0]);
	    if (segment % 2)
		frac = 1 - frac;
	    *H = (segment + frac) / 6;
	    if (*H >= 1)
		--*H;
	}
     }
}

//Datenausgabe------------------------------------------------------------------------------

void ws2801_storage_show(void)
{
	uint16_t dmxch;
	for(dmxch = 0; dmxch < ws2801_channels; dmxch++)
	{
		ws2801_writebyte(ws2801_dmxUniverse[dmxch]);
	}
    
    	if (dmxch == ws2801_channels) {
    		ws2801_showpixel();
    	}
	ws2801_state = 1;
}

void ws2801_writebyte(unsigned char Send)
{
	register unsigned char BitCount = 8; // store variable BitCount in a cpu register
	do
	{
		PIN_CLEAR(WS2801_CLOCK);	// set clock LOW
		// send bit to ws2801. we do MSB first
		if (Send & 0x80) 
		{
    		PIN_SET(WS2801_DATA); // set output HIGH
		}
		else
		{
    		PIN_CLEAR(WS2801_DATA); // set output LOW
		}
		PIN_SET(WS2801_CLOCK); // set clock HIGH
		// next bit
		Send <<= 1;
	}while (--BitCount);
} // ws2801_writeByte

void ws2801_showpixel(void) {
    // when we're done we hold the clock pin low for a millisecond to latch it
    PIN_CLEAR(WS2801_CLOCK); // set clock LOW
    _delay_us(500); // wait for 500uS to display frame on ws2801
}

//----------------------------------------------------------------------------------------------

void ws2801_state_set (uint8_t val) {
	if (ws2801_artnet_state == 0) {
             if (val != ws2801_state) {
		ws2801_state = val;
		if (ws2801_state == 0) {
			//ws2801_clear();
			if (ws2801_ramp_down_timer > 0){
			  ramp_down_timer = ws2801_ramp_down_timer*milli;
			} else {
			  ramp_down_timer = 0;
			  ws2801_clear();
			}
		} else {
			//ws2801_storage_show();
			if (ws2801_ramp_up_timer > 0){
			  ramp_up_timer = ws2801_ramp_up_timer*milli;
			} else {
			  ramp_up_timer = 0;
			  ws2801_storage_show();
			}
		}
             }
	} else if (ws2801_artnet_state == 1) {
		if (val == 0) {
			ws2801_artnet_state = 0;
			ws2801_state = 0;
		}
	}
}

void ws2801_state_toggle (void) {
	if (ws2801_artnet_state == 0) {
		ws2801_state ^= 1;
		if (ws2801_state == 0) {
			//ws2801_clear();
			if (ws2801_ramp_down_timer > 0){
			  ramp_down_timer = ws2801_ramp_down_timer*milli;
			} else {
			  ramp_down_timer = 0;
			  ws2801_clear();
			}
		} else {
			//ws2801_storage_show();
			if (ws2801_ramp_up_timer > 0){
			  ramp_up_timer = ws2801_ramp_up_timer*milli;
			} else {
			  ramp_up_timer = 0; 
			  ws2801_storage_show();
			}
		}
	}
}

void ws2801_artnet_state_set (uint8_t val) {
	ws2801_artnet_state = val;
	ws2801_state = val;
	if (ws2801_artnet_state == 0) {
		ws2801_clear();
		ws2801_storage_write();
	}
}

void ws2801_artnet_state_toggle (void) {
	ws2801_artnet_state ^= 1;
	if (ws2801_artnet_state == 1) {
		ws2801_state = 1;
	} else {
		ws2801_state = 0;
		ws2801_clear();
		ws2801_storage_write();
	}
}

void ws2801_dim_up (void) {
        ws2801_dim_state++;
	if (ws2801_dim_state == 16) {
		ws2801_dim_state = 0;
	}
}

void ws2801_dim_down (void) {
        ws2801_dim_state--;
	if (ws2801_dim_state == -1) {
		ws2801_dim_state = 15;
	}
}

void ws2801_dim_updown (void) {
	if (ws2801_dim_direction == 0) {
		ws2801_dim_state--;
		if (ws2801_dim_state == -1) {
			ws2801_dim_state = 0;
			ws2801_dim_direction = 1;
		}
	} else {
		ws2801_dim_state++;
		if (ws2801_dim_state == 16) {
			ws2801_dim_state = 15;
			ws2801_dim_direction = 0;
		}
	}	
}

void ws2801_dim_set (int dim) {
	if ((dim<16)&&(dim>0)) {
		ws2801_dim_state = dim;
	}
}

void ws2801_dim_ramp_up_set (int timer) {
	if ((timer<16000)&&(timer>0)) {
		ramp_up_timer = (uint16_t)timer*milli;
	}
}

void ws2801_dim_ramp_up_timer_set (int timer) {
	if ((timer<16000)&&(timer>0)) {
		ws2801_ramp_up_timer = (uint16_t)timer*milli;
	}
}

void ws2801_dim_ramp_down_set (int timer) {
	if ((timer<16000)&&(timer>0)) {
		ramp_down_timer = (uint16_t)timer*milli;
	}
}

void ws2801_dim_ramp_down_timer_set (int timer) {
	if ((timer<16000)&&(timer>0)) {
		ws2801_ramp_down_timer = (uint16_t)timer*milli;
	}
}

void ws2801_dim_onfor_timer_set (int timer) {
	if ((timer<16000)&&(timer>0)) {
		onfor_timer = (uint16_t)timer*milli;
	}
}

void
ws2801_periodic(void)
{
  if (ws2801_artnet_state == 0) {
  switch(periodic_state) {
    case WS2801_STATE_IDLE:
      if (ramp_up_timer > 0){
        WS2801_DEBUG("ramp_up\r\n");
	periodic_state = WS2801_STATE_RAMP_UP;
        ws2801_dim_steps = ramp_up_timer/(ws2801_dim_state - ws2801_slow_dim_state + 2);
      }
      if (ramp_down_timer > 0){
        WS2801_DEBUG("ramp_down\r\n");
	periodic_state = WS2801_STATE_RAMP_DOWN;
	ws2801_slow_dim_state = ws2801_dim_state;
        ws2801_dim_steps = ramp_down_timer/(ws2801_slow_dim_state + 2);
      }
      if (onfor_timer > 0){
        WS2801_DEBUG("on_for_timer\r\n");
	periodic_state = WS2801_STATE_ONFORTIMER;
	ws2801_state = 1;
	ws2801_storage_write();
	ws2801_storage_show();
      }
      return;

    case WS2801_STATE_RAMP_UP:
      if (ramp_up_timer > 0) {
	if (ws2801_slow_dim_state < ws2801_dim_state) {
	  if (ws2801_timer > 0) {
	  ws2801_timer--;
	  } else {
	  ws2801_timer = ws2801_dim_steps;
	  ws2801_slow_dim_state++;
	  }
	}
	  ws2801_storage_write_slowdim();
	  ws2801_storage_show();
        ramp_up_timer--;
        return;
      }

      /* reset state */
      periodic_state = WS2801_STATE_IDLE;
      ws2801_state = 1;
      return;

    case WS2801_STATE_RAMP_DOWN:
      if (ramp_down_timer > 0) {
	if (ws2801_slow_dim_state > 0) {
	  if (ws2801_timer > 0) {
	  ws2801_timer--;
	  } else {
	  ws2801_timer = ws2801_dim_steps;
	  ws2801_slow_dim_state--;
	  }
	  ws2801_storage_write_slowdim();
	  ws2801_storage_show();
	} else {
          ws2801_clear();
	}
	  //ws2801_storage_write_slowdim();
	  //ws2801_storage_show();
        ramp_down_timer--;
        return;
      }

      /* reset state */
      periodic_state = WS2801_STATE_IDLE;
      ws2801_state = 0;
      return;

    case WS2801_STATE_ONFORTIMER:
      if (onfor_timer > 0) {
        onfor_timer--;
        return;
      }

      ws2801_state_set(0);

      /* reset state */
      periodic_state = WS2801_STATE_IDLE;
      return;

    default:
      periodic_state = WS2801_STATE_IDLE;
  }
 } else {
     ws2801_out();
 }
}


#endif /* WS2801_SUPPORT */

/*
   -- Ethersex META --
   header(protocols/ws2801/ws2801.h)
   net_init(ws2801_init)
   timer(1, ws2801_periodic())
   block(Miscelleanous)
 */
