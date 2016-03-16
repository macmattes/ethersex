/*
 *
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2008,2009 by Christian Dietrich <stettberger@dokucode.de>
 *
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
 */

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>

#include "config.h"
#include "autoconf.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "i2c_master.h"
#include "i2c_mcp4725.h"

#ifdef DEBUG_I2C
#define DEBUGGI2C(fnc, msg...) debug_printf("I2C: %s: ", fnc); debug_printf(msg)
#else
#define DEBUGGI2C(fnc, msg...)
#endif

#define PowerDownBit0   0x00 //Normal Mode
#define PowerDownBit1	0x02 //1 kΩ resistor to ground (1)
#define PowerDownBit2   0x04 //100 kΩ resistor to ground (1)
#define PowerDownBit3	0x06 //500 kΩ resistor to ground (1)
//Note 1: In the power-down mode: VOUT is off and most of internal circuits are disabled.

#define CMD_FASTMODE 	0x00 //This command is used to change the DAC register. EEPROM is not affected
#define CMD_WRITEDAC	0x40 //Load configuration bits and data code to the DAC Register
#define CMD_WRITEDACEEPROM	0x60 //(a) Load configuration bits and data code to the DAC Register and (b) also write the EEPROM

#ifdef I2C_MCP4725_SUPPORT

//uint16_t i2c_mcp4725_value_dac=0;
//uint16_t i2c_mcp4725_value_eep=0;

void
i2c_mcp4725_set(uint16_t value)
{

  /*select slave in write mode */
  if (!i2c_master_select(I2C_MCP4725_ADDRESS, TW_WRITE))
    goto end;

  /*send the WRITE COMMAND */
  TWDR = CMD_WRITEDAC;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  /*send the Upper data bits (D11.D10.D9.D8.D7.D6.D5.D4) */
  TWDR = (value / 16);
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;
  /*send the Lower data bits (D3.D2.D1.D0.x.x.x.x) */
  TWDR = ((value % 16) << 4);
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

end:
  i2c_master_stop();
  return;
}

void
i2c_mcp4725_get(void)
{
  uint8_t data[5];

  /* Do an start condition */
  if (i2c_master_start() != TW_START)
    goto end;
  /*select the slave in read mode */
  TWDR = (I2C_MCP4725_ADDRESS << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

  /*get the first byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[0] = TWDR;
  /*get the second byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[1] = TWDR;
  /*get the third byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[2] = TWDR;
   /*get the fourth byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[3] = TWDR;
  /*get the fifth byte from the slave */
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[4] = TWDR;

  i2c_mcp4725_value_dac = ((data[1]<<4)|(data[2]>>4));
  i2c_mcp4725_value_eep = (((data[3]%16)<<8)|(data[4]));
  i2c_mcp4725_pdmode_dac = ((data[0]%16)>>1);
  i2c_mcp4725_pdmode_eep = ((data[3]/16)>>1);

#ifdef DEBUG_I2C
  debug_printf("i2c_mcp4725_read_bytes1-4: %hhu %hhu %hhu %hhu \n",data[1],data[2],data[3],data[4]);
  debug_printf("i2c_mcp4725_dac-eep_value: %d %d \n", i2c_mcp4725_value_dac, i2c_mcp4725_value_eep);
  debug_printf("i2c_mcp4725_dac-eep_pdmode: %d %d \n", i2c_mcp4725_pdmode_dac, i2c_mcp4725_pdmode_dac);
#endif

end:
  i2c_master_stop();
  return;
}

#endif /* I2C_MCP4725_SUPPORT */
