#include <avr/io.h>

#include "can_routines.h"
#include "uart.h"
#include "uart_master.h"

static void uart_putc_hex(char c);

void uart_put_can_msg(can_t *msg)
{
	uint8_t i;
	for(i=0;i<msg->length;i++)
	{
		uart_putc_hex(msg->data[i]);
	}
	uart_puts("\r\n");
}

static void uart_putc_hex(char c)
{
	char c1;

	c1 = (c >> 4 & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 7;
	uart_putc(c1);
	
	c1 = (c & 0x0F) + 48;
	if(c1 > 0x39)
		c1 += 7;
	uart_putc(c1);
}

void uart_master_work()
{
	static can_t msg;
	static uint8_t recv_counter = 0;
    
	char rxbyte;

	msg.flags.rtr = 0;
	msg.flags.extended = 0;

	if(!uart_data())
		return;

	rxbyte=uart_getchar();
	if(rxbyte == 0x1B) // ESC
	{
		uart_puts("reset\r\n");
		recv_counter = 0;
	}
	else
	{
		//uart_putc(rxbyte); // echo
		if(rxbyte > 0x39)
			rxbyte -= 7;
		switch(recv_counter++)
		{
			case 0:		msg.id = (rxbyte-48) << 8; break;
			case 1:		msg.id |= (rxbyte-48) << 4; break;
			case 2:		msg.id |= (rxbyte-48) << 0; break;
			
			case 3:		msg.data[0] = (rxbyte-48) << 4; break; //command
			case 4:		msg.data[0] |= (rxbyte-48) << 0; break; // command
			
			case 5:		msg.data[1] = (rxbyte-48) << 4; break; //address
			case 6:		msg.data[1] |= (rxbyte-48) << 0; break; //address
			
			case 7:		msg.length = (rxbyte-48) << 4; break; //length
			case 8:		msg.length |= (rxbyte-48) << 0; break; //length
			
			case 9:		msg.data[2] = (rxbyte-48) << 4; break;
			case 10:	msg.data[2] |= (rxbyte-48) << 0; break;
			case 11:	msg.data[3] = (rxbyte-48) << 4; break;
			case 12:	msg.data[3] |= (rxbyte-48) << 0; break;
			case 13:	msg.data[4] = (rxbyte-48) << 4; break;
			case 14:	msg.data[4] |= (rxbyte-48) << 0; break;
			case 15:	msg.data[5] = (rxbyte-48) << 4; break;
			case 16:	msg.data[5] |= (rxbyte-48) << 0; break;
			case 17:	msg.data[6] = (rxbyte-48) << 4; break;
			case 18:	msg.data[6] |= (rxbyte-48) << 0; break;
			case 19:	msg.data[7] = (rxbyte-48) << 4; break;
			case 20:	msg.data[7] |= (rxbyte-48) << 0; break;
		}

		if(recv_counter > 8 && recv_counter > (msg.length*2 + 4))
		{
			uart_puts("sending\r\n");
			recv_counter = 0;

			can_parse_msg(&msg); // parse message as it might be directly for us
			can_send_message(&msg);
		}
	}
}

