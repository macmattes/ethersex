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

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"
#include "protocols/ws2801/ws2801_net.h"
#ifndef _WS2801_H
#define _WS2801_H

#ifdef WS2801_SUPPORT
uint16_t ws2801_port;
uint8_t ws2801_artnet_universe;
uint16_t ws2801_colortemp;
uint8_t ws2801_artnet_state;
uint8_t ws2801_pixels;
uint16_t ws2801_channels;
int ws2801_dim_state;
int ws2801_slow_dim_state;


#ifdef DEBUG_WS2801
#include "core/debug.h"
#define WS2801_DEBUG(str...) debug_printf ("ws2801: " str)
#else
#define WS2801_DEBUG(...)    ((void) 0)
#endif



/* ----------------------------------------------------------------------------
 * op-codes
 */
#define OP_POLL			0x2000
#define OP_POLLREPLY		0x2100
#define OP_OUTPUT		0x5000
#define OP_ADDRESS		0x6000
#define OP_IPPROG		0xf800
#define OP_IPPROGREPLY		0xf900

/* ----------------------------------------------------------------------------
 * status
 */
#define RC_POWER_OK		0x01
#define RC_PARSE_FAIL		0x04
#define RC_SH_NAME_OK		0x06
#define RC_LO_NAME_OK		0x07

/* ----------------------------------------------------------------------------
 * default values
 */
#define SUBNET_DEFAULT		0
#define NETCONFIG_DEFAULT	1

/* ----------------------------------------------------------------------------
 * other defines
 */
#define WS2801_STATE_IDLE     	0
#define WS2801_STATE_RAMP_UP     1
#define WS2801_STATE_RAMP_DOWN   2
#define WS2801_STATE_ONFORTIMER 3
#define WS2801_STATE_ARTNET   	4

/* ----------------------------------------------------------------------------
 * packet formats
 */
struct ws2801_header
{
  uint8_t id[8];
  uint16_t opcode;
};

struct ws2801_dmx
{
  uint8_t id[8];
  uint16_t opcode;
  uint8_t versionH;
  uint8_t version;
  uint8_t sequence;
  uint8_t physical;
  uint16_t universe;
  uint8_t lengthHi;
  uint8_t length;
  uint8_t dataStart[];
};

uint8_t ws2801_state, ws2801_r, ws2801_g, ws2801_b, ws2801_dim_direction;

void ws2801_init(void);
void ws2801_sendPollReply(void);
void ws2801_main(void);
void ws2801_periodic(void);
void ws2801_get(void);
void ws2801_color_set(uint8_t r, uint8_t g, uint8_t b);
void ws2801_colortemp_set(uint16_t k);
void ws2801_clear(void);
void ws2801_storage_write(void);
void ws2801_storage_write_slowdim(void);
void ws2801_storage_show(void);
void ws2801_state_set(uint8_t val);
void ws2801_state_toggle(void);
void ws2801_artnet_state_set(uint8_t val);
void ws2801_artnet_state_toggle(void);
void ws2801_dim_up(void);
void ws2801_dim_down(void);
void ws2801_dim_updown(void);
void ws2801_dim_set(uint8_t);
void ws2801_dim_ramp_up_set(uint16_t);
void ws2801_dim_ramp_up_timer_set(uint16_t);
void ws2801_dim_ramp_down_set(uint16_t);
void ws2801_dim_ramp_down_timer_set(uint16_t);
void ws2801_dim_onfor_timer_set(uint16_t); 

void ws2801_writebyte(unsigned char);
void ws2801_showpixel(void);

void hsv_to_rgb(double h, double s, double v, double *R, double *G, double *B);
void rgb_to_hsv(double r, double g, double b, double *H, double *S, double *V);

#endif /* _WS2801_H */
#endif /* WS2801_SUPPORT */
