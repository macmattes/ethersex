/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2009 by Stefan Riepenhausen <rhn@gmx.net>
 * Copyright (c) 2012 by Erik Kunze <ethersex@erik-kunze.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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
#include <avr/pgmspace.h>
#include <util/twi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_mcp4725.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_mcp4725_dac(char *cmd, char *output, uint16_t len)
{
  uint8_t 	ret=0;
  uint16_t	val=0;
  ret = sscanf_P(cmd, PSTR("%d"), &val);
  if ((ret == 1) && (val<4096))
  {
      i2c_mcp4725_set(val);
      return ECMD_FINAL_OK;
  }
  else
  {
      i2c_mcp4725_get();
      return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), i2c_mcp4725_value_dac));
  }
}

int16_t
parse_cmd_mcp4725_eep(char *cmd, char *output, uint16_t len)
{
  i2c_mcp4725_get();
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), i2c_mcp4725_value_eep));
}



/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(mcp4725_dac, "mcp4725 dac", VALUE, Set/Get value)
  ecmd_feature(mcp4725_eep, "mcp4725 eep", VALUE, Set/Get value)
*/
