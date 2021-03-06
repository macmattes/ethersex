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
#include "ws2801.h"

#include "protocols/ecmd/ecmd-base.h"

int16_t parse_cmd_ws2801_universe(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    ws2801_artnet_universe = atoi(cmd);
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(ws2801_artnet_universe, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t parse_cmd_ws2801_artnet_state(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    ws2801_artnet_state = atoi(cmd);
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(ws2801_artnet_state, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t parse_cmd_ws2801_state(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    ws2801_state_set(atoi(cmd));
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(ws2801_state, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t parse_cmd_ws2801_settemp(char *cmd, char *output, uint16_t len)
{
  uint16_t ret=0, k=0;
  ret = sscanf_P(cmd, PSTR("%u"), &k);
  WS2801_DEBUG("input: %d, Dim:%d \r\n",k);
  if (ret==1)
  {
    ws2801_colortemp_set(k);
    return ECMD_FINAL_OK;
  }
  else
  {
    itoa(ws2801_colortemp, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}

int16_t parse_cmd_ws2801_set_rgb(char *cmd, char *output, uint16_t len)
{
    uint8_t ret=0, r=0, g=0, b=0;
    ret = sscanf_P(cmd, PSTR("%hhu %hhu %hhu"), &r, &g, &b);
    if(ret == 3)
	{
        ws2801_color_set( r, g, b);
	ws2801_storage_write();
        return ECMD_FINAL_OK;
    }
    else
	return ECMD_ERR_PARSE_ERROR;    
}


/*
  -- Ethersex META --
  block([[WS2801]] commands)
  ecmd_feature(ws2801_universe, "ws2801 artnet universe",UNIVERSE, set/get Universe to show)
  ecmd_feature(ws2801_artnet_state, "ws2801 artnet state",ARTNETSTATE, set/get ARTNET State)
  ecmd_feature(ws2801_state, "ws2801 state",STATE, set/get State)
  ecmd_feature(ws2801_settemp, "ws2801 colortemp",Temperature, set/get Color Temperature)
  ecmd_feature(ws2801_set_rgb, "ws2801 rgb",R G B, set rgb values) 
*/
