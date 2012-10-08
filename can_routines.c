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

void (*reset)( void ) = (void*)0x0000;
void (*bootloader)( void ) = (void*)0x1C00;

void can_set_relais(uint8_t to_address, uint8_t relais, uint8_t state)
{
	can_t msg;
	
	msg.id = address;
	msg.flags.rtr = 0;
	msg.flags.extended = 0;

	msg.id = 0x0F0;
	msg.length = 4;
	msg.data[0] = MSG_COMMAND_RELAIS;
	msg.data[1] = to_address;
	msg.data[2] = relais;
	msg.data[3] = state;
	can_parse_msg(&msg); // parse message as it might be directly for us
	can_send_message(&msg);
	uart_put_can_msg(&msg);
}

void can_status_powerup(void)
{
    can_t msg;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.flags.extended = 0;

	msg.length = 3;
	msg.data[0] = MSG_COMMAND_STATUS;
	msg.data[1] = address;
	msg.data[2] = MSG_STATUS_POWERUP;
	
	can_send_message(&msg);
	uart_put_can_msg(&msg);
}

void can_status_hr20(void)
{
	uint8_t time_no_data;

    can_t msg;
	msg.flags.extended = 0;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.length = 8;
	msg.data[0] = MSG_COMMAND_STATUS;
	msg.data[1] = address;
	msg.data[2] = MSG_STATUS_HR20_TEMPS;
	msg.data[3] = hr20status.data_valid | hr20status.mode << 1 | hr20status.window_open << 2;
	msg.data[4] = hr20status.tempis >> 8;
	msg.data[5] = hr20status.tempis & 0xFF;
	msg.data[6] = hr20status.tempset >> 8;
	msg.data[7] = hr20status.tempset & 0xFF;
	while(!can_send_message(&msg));
	
	if((uptime - hr20status.data_timestamp) > 255)
		time_no_data = 255;
	else
		time_no_data = uptime - hr20status.data_timestamp;

	msg.data[2] = MSG_STATUS_HR20_VALVE_VOLT;
	msg.data[3] = time_no_data;
	msg.data[4] = hr20status.valve;
	msg.data[5] = hr20status.voltage >> 8;
	msg.data[6] = hr20status.voltage & 0xFF;
	msg.data[7] = hr20status.error_code;
	while(!can_send_message(&msg));
}

void can_status_hr20_timer(void)
{
    can_t msg;
	msg.flags.extended = 0;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.length = 8;
	msg.data[0] = MSG_COMMAND_STATUS;
	msg.data[1] = address;
	msg.data[2] = MSG_STATUS_HR20_TIMER;
	msg.data[3] = hr20status.last_timer.day;
	msg.data[4] = hr20status.last_timer.slot;
	msg.data[5] = hr20status.last_timer.mode;
	msg.data[6] = hr20status.last_timer.time >> 8;
	msg.data[7] = hr20status.last_timer.time &0xFF;
	while(!can_send_message(&msg));
}

void can_status_relais(void)
{
    can_t msg;
	msg.flags.extended = 0;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.length = 4;
	msg.data[0] = MSG_COMMAND_STATUS;
	msg.data[1] = address;
	msg.data[2] = MSG_STATUS_RELAIS;
	msg.data[3] = relais_get();

	can_parse_msg(&msg); // parse message as it might be directly for us
	while(!can_send_message(&msg));
	uart_put_can_msg(&msg);
}

void can_status_relais_eeprom(void)
{
    can_t msg;
	uint8_t i;

	msg.flags.extended = 0;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.length = 5;

	i = 0;
	while(i<6)
	{
		msg.data[0] = MSG_COMMAND_STATUS;
		msg.data[1] = address;
		msg.data[2] = MSG_STATUS_EEPROM_RELAIS1 + i;
		msg.data[3] = relais_addresses[i];
		msg.data[4] = relais_relais[i];

		uart_put_can_msg(&msg);
		if(can_send_message(&msg)) // successfull
			i++;
	}
}

void can_status_uptime(void)
{
	uint16_t voltage;
	voltage = getBatteryVoltage();

    can_t msg;
	msg.flags.extended = 0;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.length = 8;
	msg.data[0] = MSG_COMMAND_STATUS;
	msg.data[1] = address;
	msg.data[2] = MSG_STATUS_UPTIME;
	msg.data[3] = ((uptime >>24) & 0x0F) | (VERSION<<4); // put version number in highest nibble
	msg.data[4] = (uptime >>16) & 0xFF;
	msg.data[5] = (uptime >>8) & 0xFF;
	msg.data[6] = uptime & 0xFF;
	msg.data[7] = voltage & 0xFF;

	can_send_message(&msg);
	uart_put_can_msg(&msg);
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
				hr20GetTimer(msg->data[2],msg->data[3]);
				break;
			case MSG_COMMAND_HR20_SET_TIMER:
				hr20SetTimer(msg->data[2],msg->data[3],msg->data[4],
					(msg->data[5] << 8) + msg->data[6]);
				break;
		}
	}
}

