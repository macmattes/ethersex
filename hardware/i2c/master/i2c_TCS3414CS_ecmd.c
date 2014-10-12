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
#include <string.h>
#include <math.h> 

#include "autoconf.h"
#include "config.h"
#include "core/debug.h"
#include "hardware/i2c/master/i2c_TCS3414CS.h"
#include "protocols/ecmd/ecmd-base.h"

int16_t
parse_cmd_tcs3414cs_color(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
  return ECMD_FINAL(snprintf_P(output, len, PSTR("r:%d g:%d b:%d c:%d"), red, green, blue, clr));
  }
}

#ifdef I2C_TCS3414CS_CALC_SUPPORT
int16_t
parse_cmd_tcs3414cs_colortemp(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), Colortemp));
}

int16_t
parse_cmd_tcs3414cs_illuminance(char *cmd, char *output, uint16_t len)
{
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), Illuminance));
}
#endif

/*
-- Ethersex META --

  block([[I2C]] (TWI))
  ecmd_feature(tcs3414cs_color, "tcs3414cs rgb", CHIPADDR, get color values)
#ifdef I2C_TCS3414CS_CALC_SUPPORT
  ecmd_feature(tcs3414cs_colortemp, "tcs3414cs colortemp", CHIPADDR, get colortemp value)
  ecmd_feature(tcs3414cs_illuminance, "tcs3414cs lux", CHIPADDR, get colortemp value)
#endif
*/
