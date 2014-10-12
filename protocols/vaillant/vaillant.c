/*
 *
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
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


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/crc16.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"

#ifdef DEBUG_VAILLANT
# include "core/debug.h"
# define VAILLANT_DEBUG(a...)  debug_printf("vaillant: " a)
#else
# define VAILLANT_DEBUG(a...)
#endif

#include "pinning.c"
#include "vaillant.h"

#define USE_USART VAILLANT_USE_USART
#define BAUD VAILLANT_BAUDRATE
#include "core/usart.h"


/* We generate our own usart init module, for our usart port */
generate_usart_init()

uint8_t state;
struct vaillant_generic_info new_data; /* This is used for all incoming messages */
struct vaillant_e8_info      vaillant_e8_data;
struct vaillant_generic_info vaillant_c0_data;
struct vaillant_generic_info vaillant_48_data;
struct vaillant_generic_info vaillant_50_data;

void
vaillant_init(void)
{
    /* Initialize the usart module */
    usart_init();

}

void
vaillant_periodic(void)
{
  new_data.chksum = 0;
  new_data.len = 0;

  if (state == VAILLANT_REQUEST_C0) {
    state = VAILLANT_REQUEST_E8;
    //usart(UDR) = "07000000440169";
    //uart_puts ("07000000440169");
    uart_puts ("12");
uart_puts ("34");
uart_puts ("56");
uart_puts ("78");
    VAILLANT_DEBUG("sent e8 command\n");
  } else if (state == VAILLANT_REQUEST_E8) {
    state = VAILLANT_REQUEST_48;
    usart(UDR) = 0x48;
    VAILLANT_DEBUG("sent 48 command\n");
  } else if (state == VAILLANT_REQUEST_48) {
    state = VAILLANT_REQUEST_50;
    usart(UDR) = 0x50;
    VAILLANT_DEBUG("sent 50 command\n");
  } else {
    state = VAILLANT_REQUEST_C0;
    usart(UDR) = 0xc0;
    VAILLANT_DEBUG("sent c0 command\n");
  }
}

int uart_putc(unsigned char c)
{                            
    usart(UDR) = c;                      /* sende Zeichen */
    return 0;
}

void uart_puts (char *s)
{
    while (*s)
    {   /* so lange *s != '\0' also ungleich dem "String-Endezeichen(Terminator)" */
        uart_putc(*s);
        s++;
    }
}


ISR(usart(USART,_RX_vect))
{
  /* Ignore errors */
  if ((usart(UCSR,A) & _BV(usart(DOR))) || (usart(UCSR,A) & _BV(usart(FE)))) {
    uint8_t v = usart(UDR);
    (void) v;
    return;
  }
  uint8_t data = usart(UDR);
  new_data.data[new_data.len++] = data;
  new_data.chksum ^= data;

  if (state == VAILLANT_REQUEST_E8) {
    if (new_data.len == 22) {
      if (new_data.data[0] == 5 && new_data.chksum == 0) {
        memcpy(&vaillant_e8_data, &new_data, sizeof(vaillant_e8_data));
        vaillant_e8_data.data[0] = 0;
        VAILLANT_DEBUG("vaillant_e8_data correct\n");
      } else {
        VAILLANT_DEBUG("got e8 cksum: %x != %x (calc)\n", new_data.data[21], 
                   new_data.chksum ^ data);
        vaillant_e8_data.data[0]++;
      }
    }
  } else if (state == VAILLANT_REQUEST_48) {
    if (new_data.len == 76) {
        memcpy(&vaillant_48_data, &new_data, sizeof(vaillant_48_data));
        VAILLANT_DEBUG("48_data ready\n");
        vaillant_48_data.data[0] = 0;
        /* FIXME: here we have to determine if there is a checksum */
    }
  } else if (state == VAILLANT_REQUEST_50) {
    if (new_data.len == 76) {
        memcpy(&vaillant_50_data, &new_data, sizeof(vaillant_48_data));
        VAILLANT_DEBUG("50_data ready\n");
        vaillant_50_data.data[0] = 0;
        /* FIXME: here we have to determine if there is a checksum */
    }
  } else {
    if (new_data.len == 76) {
      if (new_data.data[0] == 0 && new_data.chksum == 0) {
        memcpy(&vaillant_c0_data, &new_data, sizeof(vaillant_c0_data));
        VAILLANT_DEBUG("vaillant_c0_data correct\n");
        vaillant_c0_data.data[0] = 0;
      } else {
        VAILLANT_DEBUG("got c0 cksum: %x != %x (calc)\n", new_data.data[75], new_data.chksum ^ data);
        vaillant_c0_data.data[0]++;
      }
    }
  }
}

/*
  -- Ethersex META --
  header(protocols/vaillant/vaillant.h)
  init(vaillant_init)
  timer(100, vaillant_periodic())
*/
