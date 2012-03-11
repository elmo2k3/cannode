#include <avr/io.h>
#include <avr/interrupt.h>

#include "can_routines.h"
#include "protocol.h"
#include "led.h"
#include "buttons.h"
#include "relais.h"
#include "main.h"
#include "uart_master.h"
#include "eeprom.h"

void (*reset)( void ) = (void*)0x0000;

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

	can_send_message(&msg);
	uart_put_can_msg(&msg);
}

void can_status_uptime(void)
{
    can_t msg;
	msg.flags.extended = 0;
	msg.flags.rtr = 0;
	msg.id  = address; // slave to master
	msg.length = 7;
	msg.data[0] = MSG_COMMAND_STATUS;
	msg.data[1] = address;
	msg.data[2] = MSG_STATUS_UPTIME;
	msg.data[3] = (uptime >>24) & 0xFF;
	msg.data[4] = (uptime >>16) & 0xFF;
	msg.data[5] = (uptime >>8) & 0xFF;
	msg.data[6] = uptime & 0xFF;

	can_send_message(&msg);
	uart_put_can_msg(&msg);
}

void can_parse_msg(can_t *msg)
{
	if(msg->data[1] == address || msg->data[1] == 0x00)
	{
		msg->data[1] = address;
		switch(msg->data[0])
		{
			case MSG_COMMAND_PING: // send back address | TYPE
				msg->id  = address; // slave to master
				can_send_message(msg);
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

			case MSG_COMMAND_STATUS:
				can_status_relais();
				break;

			case MSG_COMMAND_EEPROM:
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
				}
				break;
		}
	}
	// parse relais status from other nodes
	else if(msg->data[0] == MSG_COMMAND_STATUS &&
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
}

