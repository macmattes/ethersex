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

#ifndef _I2C_MCP4725_H
#define _I2C_MCP4725_H

#define I2C_SLA_MCP4725 0x62  // ADDR > 0.7 VCC)
//#define I2C_SLA_MCP4725  0x63 // ADDR < 0.3 VCC)

uint16_t i2c_mcp4725_value_dac;
uint16_t i2c_mcp4725_value_eep;
uint8_t i2c_mcp4725_pdmode_dac;
uint8_t i2c_mcp4725_pdmode_eep;

void i2c_mcp4725_set(uint16_t value);
void i2c_mcp4725_get(void);

#endif /* _I2C_MCP4725_H */
