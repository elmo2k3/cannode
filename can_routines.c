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

// some macros to assemble or dissamble the can identifier
#define CAN_CMD_FROM_ID(x) ((x >>5)&0x1F)
#define CAN_ADDRESS_FROM_ID(x) (x & 0x1F)
#define CAN_ID_SET_ACK(x) (x |= (1<<10)) // set bit number 10 to one

void (*reset)( void ) = (void*)0x0000;
void (*bootloader)( void ) = (void*)0x1C00;

void can_routines_send_msg(uint8_t cmd, uint8_t to_addr, uint8_t *data, uint8_t length, uint8_t ack)
{
	can_t msg;
	uint8_t i;
	
	msg.id = (to_addr&0x1F) | ((cmd&0x1F)<<5) | ((ack&0x01)<<10);
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
	uint8_t data[2];
	data[0] = relais;
	data[1] = state;
	can_routines_send_msg(MSG_CMD_RELAIS, to_address, data,2,0);
}

void can_status_hr20(void)
{
	uint8_t time_no_data;
	uint8_t data[8];

	data[0] = hr20status.data_valid | hr20status.mode << 1 | hr20status.window_open << 2;
	data[1] = hr20status.tempis >> 8;
	data[2] = hr20status.tempis & 0xFF;
	data[3] = hr20status.tempset >> 8;
	data[4] = hr20status.tempset & 0xFF;
	can_routines_send_msg(MSG_STATUS_HR20_TEMPS, own_address, data,5,0);
	
	if((uptime - hr20status.data_timestamp) > 255)
		time_no_data = 255;
	else
		time_no_data = uptime - hr20status.data_timestamp;

	data[0] = time_no_data;
	data[1] = hr20status.valve;
	data[2] = hr20status.voltage >> 8;
	data[3] = hr20status.voltage & 0xFF;
	data[4] = hr20status.error_code;
	can_routines_send_msg(MSG_STATUS_HR20_MISC, own_address, data,5,0);
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
	uint8_t data[1];
	
	data[0] = relais_get();
	
	can_routines_send_msg(MSG_STATUS_RELAIS, own_address, data,1,0);
}

void can_status_relais_eeprom(void)
{
//	uint8_t data[5];
//	uint8_t i;
//
//	for(i=0;i<6;i++)
//	{
//		data[0] = MSG_COMMAND_STATUS;
//		//data[1] = address;
//		data[2] = MSG_STATUS_EEPROM_RELAIS1 + i;
//		data[3] = relais_addresses[i];
//		data[4] = relais_relais[i];
//		can_routines_send_msg(data,5,1);
//	}
}

void can_status_uptime(void)
{
	uint8_t voltage;
	voltage = getBatteryVoltage();
	uint8_t data[6];

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
		data[0] = VERSION;
		data[1] = (uptime >>24) & 0x0F;
		data[2] = (uptime >>16) & 0xFF;
		data[3] = (uptime >>8) & 0xFF;
		data[4] = uptime & 0xFF;
	}
	data[5] = voltage;

	can_routines_send_msg(MSG_STATUS_UPTIME, own_address, data,6,0);
}

void can_parse_msg(can_t *msg)
{
	// parse relais status from other nodes
	if(CAN_CMD_FROM_ID(msg->id) == MSG_STATUS_RELAIS)
	{
		if(msg->length != 1)
			return;
		uint8_t i,p;
		for(i=0;i<6;i++) // go through all possible relais addresses
		{
			if(CAN_ADDRESS_FROM_ID(msg->id) == relais_addresses[i])
			{
				for(p=0;p<4;p++)
				{
					if(msg->data[0] & relais_relais[i])
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
	// parse data for own address
	else if(CAN_ADDRESS_FROM_ID(msg->id) == own_address)
	{
		switch(CAN_CMD_FROM_ID(msg->id))
		{
			case MSG_CMD_BOOTLOADER:
				if(msg->length != 0)
					break;
				cli();
				bootloader();
				break;

			case MSG_CMD_RESET:
				if(msg->length != 0)
					break;
				cli();
				reset();
				break;

			case MSG_CMD_RELAIS:
				if(msg->length != 2)
					break;
				if(msg->data[0] == 0x01)
					relais_set(RELAIS_1, msg->data[1]);
				else if(msg->data[0] == 0x02)
					relais_set(RELAIS_2, msg->data[1]);
				else if(msg->data[0] == 0x04)
					relais_set(RELAIS_3, msg->data[1]);
				else if(msg->data[0] == 0x08)
					relais_set(RELAIS_4, msg->data[1]);
				break;

			case MSG_CMD_CONFIG_SET:
				if(msg->length != 2)
					break;
				switch(msg->data[0])
				{
					case CONF_NODE_ADDRESS:
						eeprom_set_address(msg->data[1]);
						break;
					case CONF_BUTTON_1_ADDRESS:
						eeprom_set_button_address(0, msg->data[1]);
						break;
					case CONF_BUTTON_1_RELAIS:
						eeprom_set_button_relais(0, msg->data[1]);
						break;
					case CONF_BUTTON_2_ADDRESS:
						eeprom_set_button_address(1, msg->data[1]);
						break;
					case CONF_BUTTON_2_RELAIS:
						eeprom_set_button_relais(1, msg->data[1]);
						break;
					case CONF_BUTTON_3_ADDRESS:
						eeprom_set_button_address(2, msg->data[1]);
						break;
					case CONF_BUTTON_3_RELAIS:
						eeprom_set_button_relais(2, msg->data[1]);
						break;
					case CONF_BUTTON_4_ADDRESS:
						eeprom_set_button_address(3, msg->data[1]);
						break;
					case CONF_BUTTON_4_RELAIS:
						eeprom_set_button_relais(3, msg->data[1]);
						break;
					case CONF_BUTTON_5_ADDRESS:
						eeprom_set_button_address(4, msg->data[1]);
						break;
					case CONF_BUTTON_5_RELAIS:
						eeprom_set_button_relais(4, msg->data[1]);
						break;
					case CONF_BUTTON_6_ADDRESS:
						eeprom_set_button_address(5, msg->data[1]);
						break;
					case CONF_BUTTON_6_RELAIS:
						eeprom_set_button_relais(5, msg->data[1]);
						break;
					case CONF_BANDGAP:
						eeprom_set_bandgap(msg->data[1]);
						break;
					case CONF_MODE:
						eeprom_set_uart_master(msg->data[1]);
						cli();
						reset();
						break;
				}
				break;
			case MSG_CMD_CONFIG_GET: // TBD
				switch(msg->data[0])
				{
					case CONF_MODE:
						break;
					case CONF_BUTTON_1_ADDRESS:
						can_status_relais_eeprom();
						break;
				}
				break;
			case MSG_HR20_SET_T:
				if(msg->length != 1)
					break;
				hr20SetTemperature(msg->data[0]);
				break;
			case MSG_HR20_SET_MODE_MANU:
				if(msg->length != 0)
					break;
				hr20SetModeManu();
				break;
			case MSG_HR20_SET_MODE_AUTO:
				if(msg->length != 0)
					break;
				hr20SetModeAuto();
				break;
			case MSG_HR20_SET_TIME_DATE:
				if(msg->length != 6)
					break;
				hr20SetTime(msg->data[0], msg->data[1], msg->data[2]);
				hr20SetDate(msg->data[3], msg->data[4], msg->data[5]);
				break;
		}
	}
}

