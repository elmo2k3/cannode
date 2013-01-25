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
#include <util/delay.h>

#include "can_routines.h"
#include "uart.h"
#include "uart_master.h"

#define UART_BAUDRATE 38400

static uint8_t uart_master_active;
static uint8_t uart_master_global_activated;

void uart_master_init(uint8_t activated)
{
//	uart_master_active = activated;
	uart_master_global_activated = activated;

	if(activated)
	{
		uart_init(UART_BAUD_SELECT(UART_BAUDRATE, F_CPU));
	}
}

void uart_put_can_msg(can_t *msg)
{
	uint8_t i;

	if(!uart_master_global_activated)
		return;
	if(!uart_master_active)
		return;

	uart_putc('t');
	uart_putc_hex(msg->id >> 8);
	uart_putc_hex(msg->id >> 0);
	uart_putc_hex(msg->length);
	for(i=0;i<msg->length;i++)
	{
		uart_putc_hex(msg->data[i]);
	}
	uart_puts("\r");
}

void uart_putc_hex(uint8_t c)
{
	char c1;

	c1 = (c >> 4 & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 39;
	uart_putc(c1);
	
	c1 = (c & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 39;
	uart_putc(c1);
}

void uart_putc_hex_XXX(uint16_t c)
{
	char c1;

	c1 = (c >> 8 & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 39;
	uart_putc(c1);
	
	c1 = (c >> 4 & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 39;
	uart_putc(c1);
	
	c1 = (c & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 39;
	uart_putc(c1);
}

uint8_t hex_to_uint8(uint8_t msb, uint8_t lsb)
{
	if(msb > 0x39) // A-F
		msb -= 7;
	if(msb > 0x46) // a-f
		msb -= 32;
	msb -= 48; // downto binary value
	if(lsb > 0x39) // A-F
		lsb -= 7;
	if(lsb > 0x46) // a-f
		lsb -= 32;
	lsb -= 48; // downto binary value
	return msb<<4 | lsb;
}

uint16_t hex_to_uint16(uint8_t msb1, uint8_t lsb1, uint8_t msb2, uint8_t lsb2)
{
	return hex_to_uint8(msb1,lsb1)<<8 | hex_to_uint8(msb2,lsb2);
}

void uart_master_work()
{
	static can_t msg;
	static uint8_t recv_counter = 0;
	static uint8_t rxbuf[30];
	char rxbyte;
	uint8_t i;
	
	if(!uart_data())
		return;
	if(!uart_master_global_activated)
		return;

	msg.flags.rtr = 0;
	msg.flags.extended = 0;
	
	while(uart_data())
	{
		rxbyte = uart_getchar();
		if(rxbyte == '\r') // work on command
		{
			switch(rxbuf[0])
			{
				case 'S':	// setup can-bitrate
					if(recv_counter != 2)
					{
						uart_putc(7);
					}
					else // success
					{
						uart_putc('S');
						uart_putc('\r');
					}
					break;
				case 's':	// setup can-bitrate manually
					uart_putc(7);
					break;
				case 'O':
					if(recv_counter != 1)
					{
						uart_putc(7);
					}
					else
					{
						uart_master_active = 1;
						uart_putc('\r');
					}
					break;
				case 'C':
					if(recv_counter != 1)
					{
						uart_putc(7);
					}
					else
					{
						uart_master_active = 0;
						uart_putc('\r');
					}
					break;
				case 't': // transmit 11bit Can Frame
					if((rxbuf[4]-48)*2 == (recv_counter-5)) // check if length is ok
					{
						msg.id = hex_to_uint16(0,rxbuf[1],rxbuf[2],rxbuf[3]);
						msg.length = rxbuf[4]-48;
						for(i=0;i<msg.length;i++)
						{
							msg.data[i] = hex_to_uint8(rxbuf[5+i*2],rxbuf[5+i*2+1]);
						}
						can_parse_msg(&msg); // parse message as it might be directly for us
						while(!can_send_message(&msg));
						uart_putc('z');
						uart_putc('\r');
					}
					else
					{
						uart_putc(7);
					}
					break;
				default:
					uart_putc(7);
			}
			recv_counter = 0;
		}
		else if(rxbyte == '\n')
		{
		}// just ignore NL
		else // sort in incoming byte
		{
			rxbuf[recv_counter++] = rxbyte;
			if(recv_counter > 29) // too much input -> error
			{
				recv_counter = 0;
				uart_putc(7); // bell for error
			}
		}

//		if(rxbyte > 0x39) // A-F
//			rxbyte -= 7;
//		if(rxbyte > 0x46) // a-f
//			rxbyte -= 32;
//		rxbyte -= 48; // downto binary value
//
//		switch(command)
//		{
//			case 'S':		// set can bitrate
//				
//		if(rxbyte == 0x1B) // ESC
//		{
//			recv_counter = 0;
//		}
//		else
//		{
//			//uart_putc(rxbyte); // echo
//			switch(recv_counter++)
//			{
//				case 0:		msg.id = (rxbyte-48) << 8; break;
//				case 1:		msg.id |= (rxbyte-48) << 4; break;
//				case 2:		msg.id |= (rxbyte-48) << 0; break;
//				
//				
//				case 3:		msg.length = (rxbyte-48) << 4; break; //length
//				case 4:		msg.length |= (rxbyte-48) << 0; break; //length
//				/*case 5:		msg.data[0] = (rxbyte-48) << 4; break; //command
//				case 6:		msg.data[0] |= (rxbyte-48) << 0; break; // command
//				
//				case 7:		msg.data[1] = (rxbyte-48) << 4; break; //address
//				case 8:		msg.data[1] |= (rxbyte-48) << 0; break; //address
//				
//				case 9:		msg.data[2] = (rxbyte-48) << 4; break;
//				case 10:	msg.data[2] |= (rxbyte-48) << 0; break;
//				case 11:	msg.data[3] = (rxbyte-48) << 4; break;
//				case 12:	msg.data[3] |= (rxbyte-48) << 0; break;
//				case 13:	msg.data[4] = (rxbyte-48) << 4; break;
//				case 14:	msg.data[4] |= (rxbyte-48) << 0; break;
//				case 15:	msg.data[5] = (rxbyte-48) << 4; break;
//				case 16:	msg.data[5] |= (rxbyte-48) << 0; break;
//				case 17:	msg.data[6] = (rxbyte-48) << 4; break;
//				case 18:	msg.data[6] |= (rxbyte-48) << 0; break;
//				case 19:	msg.data[7] = (rxbyte-48) << 4; break;
//				case 20:	msg.data[7] |= (rxbyte-48) << 0; break;*/
//				// shorter and flash saving: 
//				default:	if((recv_counter-5) % 2)
//								msg.data[(recv_counter-5-(recv_counter-5)%2)/2] = (rxbyte-48) << 0;
//							else
//								msg.data[(recv_counter-5-(recv_counter-5)%2)/2] = (rxbyte-48) << 4;
//							break;
//				// next stage:  6 bytes MORE
//				/*default:
//								msg.data[(recv_counter-5-(recv_counter-5)%2)/2] = (rxbyte-48) << ((recv_counter -4)%2)*4;
//							break;
//				*/
//			}
//
//			if(recv_counter > 8 && recv_counter > (msg.length*2 + 4))
//			{
//				recv_counter = 0;
//
//				can_parse_msg(&msg); // parse message as it might be directly for us
//				while(!can_send_message(&msg));
//			}
		//}
	}
}

