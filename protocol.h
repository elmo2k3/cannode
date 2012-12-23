/*
 * Copyright (C) 2011-2012 Bjoern Biesenbach <bjoern@bjoern-b.de>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__


enum msg_type_command
{
	MSG_COMMAND_STATUS,		// 0
	MSG_COMMAND_PING,		// 1
	MSG_COMMAND_RESET,		// 2
	MSG_COMMAND_RELAIS,		// 3
	MSG_COMMAND_EEPROM_SET,	// 4
	MSG_COMMAND_EEPROM_GET,	// 5
	MSG_COMMAND_HR20_SET_T,	// 6
	MSG_COMMAND_HR20_SET_MODE_MANU,	// 7
	MSG_COMMAND_HR20_SET_MODE_AUTO,	// 8
	MSG_COMMAND_HR20_SET_TIME,	// 9
	MSG_COMMAND_HR20_SET_DATE,	// 10
	MSG_COMMAND_MPD,// 11 not used any more
	MSG_COMMAND_GET_STATUS, // 12 0C
	MSG_COMMAND_BOOTLOADER,
	MSG_COMMAND_HR20_GET_TIMER,
	MSG_COMMAND_HR20_SET_TIMER
};

enum msg_type_eeprom
{
	MSG_EEPROM_ID,
	MSG_EEPROM_RELAIS1,
	MSG_EEPROM_RELAIS2,
	MSG_EEPROM_RELAIS3,
	MSG_EEPROM_RELAIS4,
	MSG_EEPROM_RELAIS5, // 5
	MSG_EEPROM_RELAIS6,
	MSG_EEPROM_UART_MASTER,
	MSG_EEPROM_BANDGAP // 8
};

enum msg_type_status
{
	MSG_STATUS_POWERUP			= 0,
	MSG_STATUS_RELAIS			= 1,
	MSG_STATUS_UPTIME			= 2,
	MSG_STATUS_EEPROM_RELAIS1	= 3,
	MSG_STATUS_EEPROM_RELAIS2	= 4,
	MSG_STATUS_EEPROM_RELAIS3	= 5,
	MSG_STATUS_EEPROM_RELAIS4	= 6,
	MSG_STATUS_EEPROM_RELAIS5	= 7,
	MSG_STATUS_EEPROM_RELAIS6	= 8,

	MSG_STATUS_HR20_TEMPS		= 9,
	MSG_STATUS_HR20_VALVE_VOLT	= 10,
	MSG_STATUS_HR20_AUTO_TEMPERATURE = 11,
	MSG_STATUS_HR20_EEPROM		= 12,
	MSG_STATUS_HR20_TIMER		= 13
};

enum node_features
{
	MODE_HR20 = 1,
	MODE_UART_MASTER = 2,
	MODE_HEIZUNGS_MASTER = 4,
	MODE_BLUBB_COUNTER = 8
};

#endif
