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
	MSG_STATUS_UPTIME		= 0,
	MSG_STATUS_RELAIS		= 1,
	MSG_STATUS_HR20_TEMPS	= 2,
	MSG_STATUS_HR20_MISC	= 3,
	MSG_STATUS_CONFIG		= 4,
	MSG_CMD_BOOTLOADER		= 7,
	MSG_CMD_RESET			= 8,
	MSG_CMD_RELAIS			= 9,
	MSG_CMD_CONFIG_SET		= 10,
	MSG_CMD_CONFIG_GET		= 11,
	MSG_HR20_SET_T			= 12,
	MSG_HR20_SET_MODE_MANU	= 13,
	MSG_HR20_SET_MODE_AUTO	= 14,
	MSG_HR20_SET_TIME_DATE	= 15
};

enum msg_type_config
{
	CONF_NODE_ADDRESS		= 0,
	CONF_MODE				= 1,
	CONF_BANDGAP			= 2,
	CONF_BUTTON_1_ADDRESS	= 3,
	CONF_BUTTON_1_RELAIS	= 4,
	CONF_BUTTON_2_ADDRESS	= 5,
	CONF_BUTTON_2_RELAIS	= 6,
	CONF_BUTTON_3_ADDRESS	= 7,
	CONF_BUTTON_3_RELAIS	= 8,
	CONF_BUTTON_4_ADDRESS	= 9,
	CONF_BUTTON_4_RELAIS	= 10,
	CONF_BUTTON_5_ADDRESS	= 11,
	CONF_BUTTON_5_RELAIS	= 12,
	CONF_BUTTON_6_ADDRESS	= 13,
	CONF_BUTTON_6_RELAIS	= 14,
};

enum msg_type_mode
{
	MODE_HR20 = 1,
	MODE_UART_MASTER = 2,
	MODE_HEIZUNGS_MASTER = 4,
	MODE_BLUBB_COUNTER = 8
};

#endif
