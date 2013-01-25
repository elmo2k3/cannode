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
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "can_routines.h"
#include "protocol.h"
#include "led.h"
#include "buttons.h"
#include "relais.h"
#include "main.h"
#include "uart_master.h"
#include "eeprom.h"
#include "hr20.h"
#include "adc.h"
#include "main.h"
#include "config.h"

void (*reset)( void ) = (void*)0x0000;
void (*bootloader)( void ) = (void*)0x1C00;

void can_routines_send_msg(uint8_t *data, uint8_t length, uint8_t ack_req)
{
	can_t msg;
	uint8_t i;
	
	msg.id = address;
	msg.flags.rtr = 0;
	msg.flags.extended = 0;

	msg.length = length;
	for(i=0; i<length; i++)
	{
		msg.data[i] = data[i];
	}
	can_parse_msg(&msg); // parse message as it might be directly for us
	uart_put_can_msg(&msg);
	
	while(!can_send_message(&msg));
}

void can_set_relais(uint8_t to_address, uint8_t relais, uint8_t state)
{
	uint8_t data[4];
	data[0] = MSG_COMMAND_RELAIS;
	data[1] = to_address;
	data[2] = relais;
	data[3] = state;
	can_routines_send_msg(data,4,1);
}

void can_status_powerup(void)
{
	uint8_t data[3];
	data[0] = MSG_COMMAND_STATUS;
	data[1] = address;
	data[2] = MSG_STATUS_POWERUP;
	can_routines_send_msg(data,3,0);
}

void can_status_hr20(void)
{
	uint8_t time_no_data;
	uint8_t data[8];

	data[0] = MSG_COMMAND_STATUS;
	data[1] = address;
	data[2] = MSG_STATUS_HR20_TEMPS;
	data[3] = hr20status.data_valid | hr20status.mode << 1 | hr20status.window_open << 2;
	data[4] = hr20status.tempis >> 8;
	data[5] = hr20status.tempis & 0xFF;
	data[6] = hr20status.tempset >> 8;
	data[7] = hr20status.tempset & 0xFF;
	can_routines_send_msg(data,8,1);
	
	if((uptime - hr20status.data_timestamp) > 255)
		time_no_data = 255;
	else
		time_no_data = uptime - hr20status.data_timestamp;

	data[2] = MSG_STATUS_HR20_VALVE_VOLT;
	data[3] = time_no_data;
	data[4] = hr20status.valve;
	data[5] = hr20status.voltage >> 8;
	data[6] = hr20status.voltage & 0xFF;
	data[7] = hr20status.error_code;
	can_routines_send_msg(data,8,1);
}

//void can_status_hr20_timer(void)
//{
//    can_t msg;
//	msg.flags.extended = 0;
//	msg.flags.rtr = 0;
//	msg.id  = address; // slave to master
//	msg.length = 8;
//	msg.data[0] = MSG_COMMAND_STATUS;
//	msg.data[1] = address;
//	msg.data[2] = MSG_STATUS_HR20_TIMER;
//	msg.data[3] = hr20status.last_timer.day;
//	msg.data[4] = hr20status.last_timer.slot;
//	msg.data[5] = hr20status.last_timer.mode;
//	msg.data[6] = hr20status.last_timer.time >> 8;
//	msg.data[7] = hr20status.last_timer.time &0xFF;
//	while(!can_send_message(&msg));
//}

void can_status_relais(void)
{
	uint8_t data[4];
	
	data[0] = MSG_COMMAND_STATUS;
	data[1] = address;
	data[2] = MSG_STATUS_RELAIS;
	data[3] = relais_get();
	
	can_routines_send_msg(data,4,0);
}

void can_status_relais_eeprom(void)
{
	uint8_t data[5];
	uint8_t i;

	for(i=0;i<6;i++)
	{
		data[0] = MSG_COMMAND_STATUS;
		data[1] = address;
		data[2] = MSG_STATUS_EEPROM_RELAIS1 + i;
		data[3] = relais_addresses[i];
		data[4] = relais_relais[i];
		can_routines_send_msg(data,5,1);
	}
}

void can_status_uptime(void)
{
	uint8_t voltage;
	voltage = getBatteryVoltage();
	uint8_t data[8];

	data[0] = MSG_COMMAND_STATUS;
	data[1] = address;
	data[2] = MSG_STATUS_UPTIME;
	if(mode & MODE_BLUBB_COUNTER)
	{
#ifdef _WITH_BLUBB_COUNTER_
		data[3] = ((0 >>24) & 0x0F) | (VERSION<<4); // put version number in highest nibble
		data[4] = (0 >>16) & 0xFF;
		data[5] = (adc_blubb_value >>8) & 0xFF;
		data[6] = adc_blubb_value & 0xFF;
#endif
	}
	else
	{
		data[3] = ((uptime >>24) & 0x0F) | (VERSION<<4); // put version number in highest nibble
		data[4] = (uptime >>16) & 0xFF;
		data[5] = (uptime >>8) & 0xFF;
		data[6] = uptime & 0xFF;
	}
	data[7] = voltage;

	can_routines_send_msg(data,8,0);
	can_routines_send_msg(data,8,0);
}

void can_parse_msg(can_t *msg)
{
	// parse relais status from other nodes
	if(msg->data[0] == MSG_COMMAND_STATUS &&
			msg->data[2] == MSG_STATUS_RELAIS)
	{
		uint8_t i,p;
		for(i=0;i<6;i++) // go through all possible relais addresses
		{
			if(msg->data[1] == relais_addresses[i])
			{
				for(p=0;p<4;p++)
				{
					if(msg->data[3] & relais_relais[i])
					{
						switch(i)
						{
							case 0:	LED0_ON(); break;
							case 1:	LED1_ON(); break;
							case 2:	LED2_ON(); break;
							case 3:	LED3_ON(); break;
							case 4:	LED4_ON(); break;
							case 5:	LED5_ON(); break;
						}
					}
					else
					{
						switch(i)
						{
							case 0:	LED0_OFF(); break;
							case 1:	LED1_OFF(); break;
							case 2:	LED2_OFF(); break;
							case 3:	LED3_OFF(); break;
							case 4:	LED4_OFF(); break;
							case 5:	LED5_OFF(); break;
						}
					}
				}
			}
		}
	}
	// parse data for own address or multicast address (0x00)
	else if(msg->data[1] == address || msg->data[1] == 0x00)
	{
		msg->data[1] = address;
		switch(msg->data[0])
		{
			case MSG_COMMAND_PING: // send back address | TYPE
				msg->id  = address; // slave to master
				can_send_message(msg);
				break;
			
			case MSG_COMMAND_BOOTLOADER:
				cli();
				bootloader();
				break;

			case MSG_COMMAND_RESET:
				cli();
				reset();
				break;

			case MSG_COMMAND_RELAIS:
				if(msg->data[2] == 0x01)
					relais_set(RELAIS_1, msg->data[3]);
				else if(msg->data[2] == 0x02)
					relais_set(RELAIS_2, msg->data[3]);
				else if(msg->data[2] == 0x04)
					relais_set(RELAIS_3, msg->data[3]);
				else if(msg->data[2] == 0x08)
					relais_set(RELAIS_4, msg->data[3]);
				break;

			case MSG_COMMAND_GET_STATUS:
				can_status_relais();
				hr20_request_status();
				break;

			case MSG_COMMAND_EEPROM_SET:
				switch(msg->data[2])
				{
					case MSG_EEPROM_ID:	
						eeprom_set_address(msg->data[3]);
						can_status_powerup();
						break;
					case MSG_EEPROM_RELAIS1:
						eeprom_set_relais(0, msg->data[3], msg->data[4]);
						break;
					case MSG_EEPROM_RELAIS2:
						eeprom_set_relais(1, msg->data[3], msg->data[4]);
						break;
					case MSG_EEPROM_RELAIS3:
						eeprom_set_relais(2, msg->data[3], msg->data[4]);
						break;
					case MSG_EEPROM_RELAIS4:
						eeprom_set_relais(3, msg->data[3], msg->data[4]);
						break;
					case MSG_EEPROM_RELAIS5:
						eeprom_set_relais(4, msg->data[3], msg->data[4]);
						break;
					case MSG_EEPROM_RELAIS6:
						eeprom_set_relais(5, msg->data[3], msg->data[4]);
						break;
					case MSG_EEPROM_BANDGAP:
						eeprom_set_bandgap(msg->data[3]);
						break;
					case MSG_EEPROM_UART_MASTER:
						eeprom_set_uart_master(msg->data[3]);
						cli();
						reset();
						break;
				}
				break;
			case MSG_COMMAND_EEPROM_GET:
				can_status_relais_eeprom();
				break;
			case MSG_COMMAND_HR20_SET_T:
				hr20SetTemperature(msg->data[2]);
				break;
			case MSG_COMMAND_HR20_SET_MODE_MANU:
				hr20SetModeManu();
				break;
			case MSG_COMMAND_HR20_SET_MODE_AUTO:
				hr20SetModeAuto();
				break;
			case MSG_COMMAND_HR20_SET_TIME:
				hr20SetTime(msg->data[2], msg->data[3], msg->data[4]);
				break;
			case MSG_COMMAND_HR20_SET_DATE:
				hr20SetDate(msg->data[2], msg->data[3], msg->data[4]);
				break;
			case MSG_COMMAND_HR20_GET_TIMER:
			//	hr20GetTimer(msg->data[2],msg->data[3]);
				break;
			case MSG_COMMAND_HR20_SET_TIMER:
			//	hr20SetTimer(msg->data[2],msg->data[3],msg->data[4],
			//		(msg->data[5] << 8) + msg->data[6]);
				break;
		}
	}
}

