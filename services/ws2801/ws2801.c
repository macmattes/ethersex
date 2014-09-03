/*
 *
 * Copyright (c) 2011-2012 by Maximilian Güntner <maximilian.guentner@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
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
 */

/* Module description: ws2801 is a forwarder of ethersex dmx information to various i2c PWM chips*/
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "ws2801.h"
#include "core/debug.h"
#include "core/bool.h"
#include "services/dmx-storage/dmx_storage.h"
int8_t dmx_conn_id = -1;
uint8_t dmx_connected = FALSE;
uint8_t ws2801;

void
ws2801_init(void)
{
  //Connect to dmx-storage
  dmx_conn_id = dmx_storage_connect(WS2801_UNIVERSE);
  if (dmx_conn_id != -1)
    dmx_connected = TRUE;
  else
    dmx_connected = FALSE;

  ws2801 = 1;
  ws2801_clear();
}

void ws2801_writeByte(unsigned char Send)
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

void ws2801_showPixel(void) {
    // when we're done we hold the clock pin low for a millisecond to latch it
    PIN_CLEAR(WS2801_CLOCK); // set clock LOW
    _delay_us(500); // wait for 500uS to display frame on ws2801
}

void
ws2801_main(void)
{
  if ((get_dmx_slot_state(WS2801_UNIVERSE, dmx_conn_id)
      == DMX_NEWVALUES)&&(ws2801 == 1))
  {
   uint8_t tmp = 0;
   for(uint8_t i = 0; i < WS2801_CHANNELS; i++)
	{
	   tmp =
              get_dmx_channel_slot(WS2801_UNIVERSE,
                             i + WS2801_OFFSET,
                             dmx_conn_id);
	   ws2801_writeByte((uint8_t)tmp);
	}
    ws2801_showPixel();
  }
}

void
ws2801_clear(void)
{
   for(uint8_t i = 0; i < WS2801_CHANNELS; i++)
	{
	   ws2801_writeByte(0);
	}
    ws2801_showPixel();
}
//mainloop(ws2801_main)//timer(20,ws2801_main())
/*
   -- Ethersex META --
   header(services/ws2801/ws2801.h)
   mainloop(ws2801_main)
   init(ws2801_init)
 */
