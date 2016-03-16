/*
 * Copyright (c) 2009 by Christian Dietrich <stettberger@dokucode.de>
 * Copyright (c) 2010 by Justin Otherguy <justin@justinotherguy.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

// httplog_ecmd.c
//
// this is a literal copy of twitter_ecmd.c with "twitter" replaced by "httplog"

#include <avr/pgmspace.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "core/eeprom.h"
#include "config.h"
#include "mq135.h"
#include "protocols/ecmd/ecmd-base.h"


uint16_t
parse_cmd_mq135_ppm(char *cmd, char *output, uint16_t len)
{
    ltoa(mq135_getppm(), output, 10);
    return ECMD_FINAL(strlen(output));
}

uint16_t 
parse_cmd_mq135_calibrate(char *cmd, char *output, uint16_t len)
{
    ltoa(mq135_calibrate(), output, 10);
    return ECMD_FINAL(strlen(output));
}

uint16_t 
parse_cmd_mq135_defro(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    mq135_defaultro = atol(cmd);
    return ECMD_FINAL_OK;
  }
  else
  {
    ltoa(mq135_defaultro, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

uint16_t 
parse_cmd_mq135_readeprom(char *cmd, char *output, uint16_t len)
{
    mq135_readeep();
    return ECMD_FINAL_OK;
}

uint16_t 
parse_cmd_mq135_writeeprom(char *cmd, char *output, uint16_t len)
{
    mq135_writeeep;
    return ECMD_FINAL_OK;
}
/*
  -- Ethersex META --
  block([[MQ135]] commands)
  ecmd_feature(mq135_ppm, "mq135 ppm",, get the ppm concentration)
  ecmd_feature(mq135_calibrate, "mq135 calibrate",, run calibration of defaultro)
  ecmd_feature(mq135_defro, "mq135 defaultro",, get/set the default ro value)
  ecmd_feature(mq135_readeprom, "mq135 param read",, read tank parameter from EEPROM)
  ecmd_feature(mq135_writeeprom, "mq135 param save",, write tank parameter to EEPROM)
*/
