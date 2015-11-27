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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include "config.h"
#include "core/bool.h"
#include "core/bit-macros.h"
#include "protocols/ws2801/ws2801.h"
#include "protocols/uip/uip.h"
#include "protocols/uip/uip_router.h"
#include "protocols/ecmd/ecmd-base.h"
#include "services/dmx-storage/dmx_storage.h"
#ifdef WS2801_SUPPORT

//#define BAUD 250000
//#define MAX_CHANNELS 			512

/* ----------------------------------------------------------------------------
 *global variables
 */

uint8_t ws2801_subNet = SUBNET_DEFAULT;
uint8_t ws2801_outputUniverse;
uint16_t ws2801_colortemp = 2600;
uint8_t ws2801_sendPollReplyOnChange = TRUE;
uip_ipaddr_t ws2801_pollReplyTarget;
uint32_t ws2801_pollReplyCounter = 0;
uint8_t ws2801_status = RC_POWER_OK;
char ws2801_shortName[18] = { '\0' };
char ws2801_longName[64] = { '\0' };

uint16_t ws2801_port = CONF_WS2801_PORT;
uint8_t ws2801_netConfig = NETCONFIG_DEFAULT;

volatile unsigned char  ws2801_dmxUniverse[512];
volatile uint16_t ws2801_artnetChannels = 0;
uint8_t ws2801_pixels = CONF_WS2801_PIXELS;
uint16_t ws2801_channels = CONF_WS2801_PIXELS*3;

const char ws2801_ID[8] PROGMEM = "Art-Net";
uint8_t ws2801_artnet_state = 0;

const uint16_t pwmtable_8D[32] PROGMEM =
{
    0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11, 13, 16, 19, 23,
    27, 32, 38, 45, 54, 64, 76, 91, 108, 128, 152, 181, 215, 255
};

/* ----------------------------------------------------------------------------
 * initialization of network settings
 */
void
ws2801_netInit(void)
{
  ws2801_net_init();
}

/* ----------------------------------------------------------------------------
 * initialization of Art-Net
 */
void
ws2801_init(void)
{

  WS2801_DEBUG("Init\n");
  /* read Art-Net port */
  ws2801_port = CONF_WS2801_PORT;
  /* read netconfig */
  ws2801_netConfig = NETCONFIG_DEFAULT;

  /* read subnet */
  ws2801_subNet = SUBNET_DEFAULT;
  ws2801_outputUniverse = CONF_WS2801_UNIVERSE;
  ws2801_colortemp = 2700;
  strcpy_P(ws2801_shortName, PSTR("e6ArtNode"));
  strcpy_P(ws2801_longName, PSTR("e6ArtNode hostname: " CONF_HOSTNAME));

  uip_ipaddr_copy(ws2801_pollReplyTarget,all_ones_addr);

  /* net_init */
  ws2801_netInit();

  /* annouce that we are here  */
  WS2801_DEBUG("send PollReply\n");
  ws2801_sendPollReply();

  /* enable PollReply on changes */
  ws2801_sendPollReplyOnChange = TRUE;

  /* enable ws2801 */
  PIN_CLEAR(WS2801_CLOCK);
  PIN_CLEAR(WS2801_DATA);

#ifdef WS2801_ARTNET_STARTUP
  /* enable artnet */
  ws2801_artnet_state = 1;
#endif

  /* set startcolor */
  ws2801_setColorTemp(ws2801_colortemp);

  WS2801_DEBUG("init complete\n");
  return;
}

static void
ws2801_send(uint16_t len)
{
  uip_udp_conn_t ws2801_conn;
  uip_ipaddr_copy(ws2801_conn.ripaddr, ws2801_pollReplyTarget);
  ws2801_conn.rport = HTONS(ws2801_port);
  ws2801_conn.lport = HTONS(ws2801_port);
  uip_udp_conn = &ws2801_conn;

  uip_slen = len;
  uip_process(UIP_UDP_SEND_CONN);
  router_output();

  uip_slen = 0;
}


/* ----------------------------------------------------------------------------
 * send an ArtPollReply packet
 */
void
ws2801_sendPollReply(void)
{

  /* prepare ws2801 PollReply packet */
  struct ws2801_pollreply *msg =
    (struct ws2801_pollreply *) &uip_buf[UIP_LLH_LEN + UIP_IPUDPH_LEN];
  memset(msg, 0, sizeof(struct ws2801_pollreply));
  WS2801_DEBUG("PollReply allocated\n");
  strncpy_P((char *) msg->id, ws2801_ID, 8);

  msg->opcode = OP_POLLREPLY;

  msg->versionInfoH = (FIRMWARE_VERSION >> 8) & 0xFF;
  msg->versionInfo = FIRMWARE_VERSION & 0xFF;

  msg->subSwitchH = 0;
  msg->subSwitch = ws2801_subNet & 15;

  /* Report as 'AvrArtNode' http://www.dmxcontrol.de/wiki/Art-Net-Node_f%C3%BCr_25_Euro */
  msg->oem = 0x08b1;
  msg->ubeaVersion = 0;
  msg->status = 0;
  /* Report as Manufacturer "ESTA" http://tsp.plasa.org/tsp/working_groups/CP/mfctrIDs.php */
  msg->estaMan = 0xFFFF;
  strcpy(msg->shortName, ws2801_shortName);
  strcpy(msg->longName, ws2801_longName);
  sprintf(msg->nodeReport, "#%04X [%04u] e6ArtNode is ready", ws2801_status,
          (unsigned int) ws2801_pollReplyCounter);

  msg->numPortsH = 0;
  msg->numPorts = 1;

  msg->portTypes[0] = PORT_TYPE_DMX_OUTPUT;
  msg->goodInput[0] = (1 << 3);
  msg->goodOutput[0] = (1 << 1);
 
  msg->swout[0] = (ws2801_subNet & 15) * 16 | (ws2801_outputUniverse & 15);
  msg->style = STYLE_NODE;

  memcpy(msg->mac, uip_ethaddr.addr, 6);

  /* broadcast the packet */
  ws2801_send(sizeof(struct ws2801_pollreply));
}

int16_t
parse_cmd_ws2801_pollreply(int8_t * cmd, int8_t * output, uint16_t len)
{
  ws2801_sendPollReply();
  return ECMD_FINAL_OK;
}

void
processPollPacket(struct ws2801_poll *poll)
{
  if ((poll->talkToMe & 2) == 2)
    ws2801_sendPollReplyOnChange = TRUE;
  else
    ws2801_sendPollReplyOnChange = FALSE;
  if ((poll->talkToMe & 1) == 1)
	  uip_ipaddr_copy(ws2801_pollReplyTarget,uip_hostaddr);
  else
	  uip_ipaddr_copy(ws2801_pollReplyTarget,all_ones_addr);
  ws2801_sendPollReply();
}

/* ----------------------------------------------------------------------------
 * receive Art-Net packet
 */
void
ws2801_get(void)
{
  struct ws2801_header *header;

  header = (struct ws2801_header *) uip_appdata;

  /* check the id */
  if (strncmp_P((char *) header->id, ws2801_ID, 8))
  {
    WS2801_DEBUG("Wrong ArtNet header, discarded\r\n");
    ws2801_status = RC_PARSE_FAIL;
    return;
  }
  switch (header->opcode)
  {
    case OP_POLL:;
      struct ws2801_poll *poll;

      WS2801_DEBUG("Received ws2801 poll packet!\r\n");
      poll = (struct ws2801_poll *) uip_appdata;
      processPollPacket(poll);
      break;
    case OP_POLLREPLY:;
      WS2801_DEBUG("Received ws2801 poll reply packet!\r\n");
      break;
    case OP_OUTPUT:;
      struct ws2801_dmx *dmx;

      WS2801_DEBUG("Received ws2801 output packet!\r\n");
      dmx = (struct ws2801_dmx *) uip_appdata;
     if (ws2801_artnet_state == 1) 
     {
      if (dmx->universe == ((ws2801_subNet << 4) | ws2801_outputUniverse))
      {
	  ws2801_artnetChannels = (dmx->lengthHi << 8) | dmx->length;
          memcpy((unsigned char*)&ws2801_dmxUniverse[0], &(dmx->dataStart), ws2801_artnetChannels);
          if (ws2801_sendPollReplyOnChange == TRUE)
          {
            ws2801_pollReplyCounter++;
            ws2801_sendPollReply();
          }
	  ws2801_show_storage();  //storage ausgeben
      }
    }
      break;
    case OP_ADDRESS:;
    case OP_IPPROG:;
      break;
    default:
      WS2801_DEBUG("Received an invalid ws2801 packet!\r\n");
      break;
  }
}

void ws2801_setColor(uint8_t r, uint8_t g, uint8_t b)
{
	ws2801_r = r;
	ws2801_g = g;
	ws2801_b = b;
}

void ws2801_setColorTemp(uint16_t k)
{
  if (ws2801_artnet_state == 0) {
    float Temperature,Red,Green,Blue;
    Temperature = k / 100;
    
    //Calculate Red:

    if (Temperature <= 66){
        Red = 255;
    } else {
        Red = Temperature - 60;
        Red = 329.698727446 * pow(Red,-0.1332047592);
        if (Red < 0) {
		Red = 0;
	}
        if (Red > 255) {
		Red = 255;
	}
    }
    
    //Calculate Green:

    if (Temperature <= 66){
        Green = Temperature;
        Green = 99.4708025861 * log(Green) - 161.1195681661;
        if (Green < 0){
		Green = 0;
	}
        if (Green > 255){
		Green = 255;
    	}
    } else {
        Green = Temperature - 60;
        Green = 288.1221695283 * pow(Green,-0.0755148492);
        if (Green < 0){
		Green = 0;
	}
        if (Green > 255) {
		Green = 255;
	}
    }
    
    //Calculate Blue:

    if (Temperature >= 66){
        Blue = 255;
    } else {
        if (Temperature <= 19) {
            Blue = 0;
        } else {
            Blue = Temperature - 10;
            Blue = 138.5177312231 * log(Blue) - 305.0447927307;
            if (Blue < 0) {
		 Blue = 0;
	    }
            if (Blue > 255) {
		Blue = 255;
	    }
        }

    }
    ws2801_setColor(Red,Green,Blue);
  }
}

void ws2801_Clear(void)
{
  if (ws2801_artnet_state == 0) {
	uint16_t dmxpx;
	for(dmxpx = 0; dmxpx < ws2801_pixels; dmxpx++)
	{
		ws2801_dmxUniverse[(dmxpx*3)+0] = 0;
		ws2801_dmxUniverse[(dmxpx*3)+1] = 0;
		ws2801_dmxUniverse[(dmxpx*3)+2] = 0;
	}
    	if (dmxpx == ws2801_pixels) {
    		ws2801_show_storage();
		ws2801_state = 0;
    	}
  }
}

void ws2801_WriteColor(void)
{
  if (ws2801_artnet_state == 0) {
	uint16_t dmxpx;
	for(dmxpx = 0; dmxpx < ws2801_pixels; dmxpx++)
	{
		ws2801_dmxUniverse[(dmxpx*3)+0] = ws2801_r;
		ws2801_dmxUniverse[(dmxpx*3)+1] = ws2801_g;
		ws2801_dmxUniverse[(dmxpx*3)+2] = ws2801_b;
	}
    	if (dmxpx == ws2801_pixels) {
    		ws2801_show_storage();
		ws2801_state = 1;
    	}
  }
}

void ws2801_Dim(uint8_t dim)
{ 
   ws2801_dim_state = dim;
	if (ws2801_artnet_state == 0) {
		if (ws2801_dim_state == 0) {
	  		ws2801_Clear();
		} else {
			double r,g,b,H,S,V;
			rgb_to_hsv(ws2801_r,ws2801_g,ws2801_b,&H,&S,&V);
			printf("HSV: %d %d %d\n",(uint8_t)H,(uint8_t)S,(uint8_t)V);
			hsv_to_rgb(H,S,pgm_read_word (& pwmtable_8D[ws2801_dim_state]),&r,&g,&b);
			printf("rgb: %d %d %d\n",(uint8_t)r,(uint8_t)g,(uint8_t)b);

			uint16_t dmxpx;
			for(dmxpx = 0; dmxpx < ws2801_pixels; dmxpx++)
			{
				ws2801_dmxUniverse[(dmxpx*3)+0] = (uint8_t) r;
				ws2801_dmxUniverse[(dmxpx*3)+1] = (uint8_t) g;
				ws2801_dmxUniverse[(dmxpx*3)+2] = (uint8_t) b;
			}
		    	if (dmxpx == ws2801_pixels) {
		    		ws2801_show_storage();
				ws2801_state = 1;
		    	}
		}
	  }
}

void
hsv_to_rgb(double h, double s, double v, double *R, double *G, double *B)
{
    int segment;
    double *rgb[3], major, minor, middle, frac;

    rgb[0] = R;
    rgb[1] = G;
    rgb[2] = B;

    while (h < 0)
	h++;
    while (h >= 1)
	h--;

    segment = (int)(h*6);

    frac = (6*h)-segment;
    if (segment % 2)
	frac = 1 - frac;

    major = v;
    minor = (1-s)*v;
    middle = frac * major + (1-frac) * minor;

    *rgb[(segment+1)/2 % 3] = major;
    *rgb[(segment+4)/2 % 3] = minor;
    *rgb[(7-segment) % 3] = middle;
}

#define swap(a,b) (temp = (a), (a) = (b), (b) = temp)
#define rshift(a,b,c) (temp = (c), (c) = (b), (b) = (a), (a) = temp)
void
rgb_to_hsv(double r, double g, double b, double *H, double *S, double *V)
{
    int segment;
    double *rgb[3], *temp, frac;

    rgb[0] = &r;
    rgb[1] = &g;
    rgb[2] = &b;

    /* sort rgb into increasing order */

    if (*rgb[0] > *rgb[1])
	swap(rgb[0], rgb[1]);
    if (*rgb[1] > *rgb[2])
	if (*rgb[0] > *rgb[2])
	    rshift(rgb[0], rgb[1], rgb[2]);
	else
	    swap(rgb[1], rgb[2]);

    if (*V = *rgb[2])
	if (*S = 1 - *rgb[0] / *rgb[2]) {
	    /* segment = 3*(b>g) + 2 * (rgb[1]==&b) + (rgb[1]==&r); */
	    segment = 3 * (rgb[0]==&g||rgb[2]==&b)
		    + 2 * (rgb[1]==&b) + (rgb[1]==&r);
	    frac = (*rgb[1] - *rgb[0]) / (*rgb[2] - *rgb[0]);
	    if (segment % 2)
		frac = 1 - frac;
	    *H = (segment + frac) / 6;
	    if (*H >= 1)
		--*H;
	}
}
//Datenausgabe

void ws2801_show_storage(void)
{
	uint16_t dmxch;
	for(dmxch = 0; dmxch < ws2801_channels; dmxch++)
	{
		ws2801_writeByte(ws2801_dmxUniverse[dmxch]);
	}
    
    	if (dmxch == ws2801_channels) {
    		ws2801_showPixel();
    	}
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

void ws2801_set_state (uint8_t val) {
	if (ws2801_artnet_state == 0) {
		ws2801_state = val;
		//ws2801_setColor(ws2801_state*ws2801_r,ws2801_state*ws2801_g,ws2801_state*ws2801_b);
		if (ws2801_state == 0) {
			ws2801_Clear();
		} else {
			ws2801_WriteColor();
		}
	} else {
		ws2801_state = 0;
	}
}

void ws2801_toggle_state (void) {
	if (ws2801_artnet_state == 0) {
		ws2801_state ^= 1;
		ws2801_setColor(ws2801_state*ws2801_r,ws2801_state*ws2801_g,ws2801_state*ws2801_b);
	}
}

void ws2801_set_artnet_state (uint8_t val) {
	ws2801_artnet_state = val;
	if (ws2801_artnet_state == 1) {
		ws2801_state = 0;
	} else {
		ws2801_Clear();
	}
}

void ws2801_toggle_artnet_state (void) {
	ws2801_artnet_state ^= 1;
	if (ws2801_artnet_state == 1) {
		ws2801_state = 0;
	} else {
		ws2801_Clear();
	}
}

#endif /* WS2801_SUPPORT */

/*
   -- Ethersex META --
   header(protocols/ws2801/ws2801.h)
   net_init(ws2801_init)
   block(Miscelleanous)
 */
