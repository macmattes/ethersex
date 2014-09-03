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

uint16_t parse_cmd_ws2801_state(char *cmd, char *output, uint16_t len)
{
  if (cmd[0])
  {
    if (atol(cmd) == 1) {
    	ws2801 = 1;
    	return ECMD_FINAL_OK;
    }
    if (atol(cmd) == 0) {
    	ws2801 = 0;
	ws2801_clear();
    	return ECMD_FINAL_OK;
    }
  }
  else
  {
    ltoa(ws2801, output, 10);
    return ECMD_FINAL(strlen(output));
  }
}


/*
  -- Ethersex META --
  block([[WS2801]] commands)
  ecmd_feature(ws2801_state, "ws2801",, enable/disable ws2801)
*/
