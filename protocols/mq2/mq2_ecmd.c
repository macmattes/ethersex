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

#include "config.h"
#include "mq2.h"

#include "protocols/ecmd/ecmd-base.h"


int16_t
parse_cmd_mq2_ppm(char *cmd, char *output, uint16_t len)
{
  int16_t ret = (int16_t)mq2_ppm;
  if (ret == 0)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));
#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("mq2 ppm %d"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), ret));
#endif
}

uint16_t
parse_cmd_mq2_ro(char *cmd, char *output, uint16_t len)
{
  long ret = mq2_getro(mq2_getres(mq2_adc),MQ2_DEFAULTPPM);
  if (ret == 0)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));
#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("mq2 ro %d"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%d"), ret));
#endif
}

int16_t
parse_cmd_mq2_res(char *cmd, char *output, uint16_t len)
{
  long ret = mq2_getres(mq2_adc);
  if (ret == 0)
    return
      ECMD_FINAL(snprintf_P(output, len, PSTR("error reading from sensor")));
#ifdef ECMD_MIRROR_REQUEST
  return ECMD_FINAL(snprintf_P(output, len, PSTR("mq2 ppm %u"), ret));
#else
  return ECMD_FINAL(snprintf_P(output, len, PSTR("%u"), ret));
#endif
}


uint16_t parse_cmd_mq2_defro(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    mq2_defaultro = atol(cmd);
    return ECMD_FINAL_OK;
  }
  else
  {
    ltoa(mq2_defaultro, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}


/*
  -- Ethersex META --
  block([[MQ2]] commands)
  ecmd_feature(mq2_ppm, "mq2 ppm",, get the ppm concentration)
  ecmd_feature(mq2_ro, "mq2 ro",, get the measured ro value)
  ecmd_feature(mq2_res, "mq2 res",, get the measured ro value)
  ecmd_feature(mq2_defro, "mq2 defaultro",, get/set the default ro value)
*/
