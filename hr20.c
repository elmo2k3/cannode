/*
 * Copyright (C) 2009-2010 Bjoern Biesenbach <bjoern@bjoern-b.de>
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

/*!
* \file hr20.c
* \brief    functions to communicate with openhr20
* \author   Bjoern Biesenbach <bjoern at bjoern-b dot de>
*/

#include <inttypes.h>
#include <string.h>
#include "protocol.h"
#include "main.h"
#include "uart.h"
#include "uart_master.h"
#include "can_routines.h"
#include "hr20.h"

#define UART_BAUDRATE 9600

//#include "hr20.h"

struct _hr20status hr20status, hr20statusTemp;
static uint8_t hr20_active;

static uint8_t hexCharToInt(char c);
static int hr20checkPlausibility(void);

static int hr20checkPlausibility()
{
    if(hr20statusTemp.tempis < 500 || hr20statusTemp.tempis > 4000)
        return 0;
    if(hr20statusTemp.tempset < 500 || hr20statusTemp.tempset > 3000)
        return 0;
    if(hr20statusTemp.valve < 0 || hr20statusTemp.valve > 100)
        return 0;
    if(hr20statusTemp.voltage < 2000 || hr20statusTemp.voltage > 4000)
        return 0;

	hr20status.tempis = hr20statusTemp.tempis;
	hr20status.tempset = hr20statusTemp.tempset;
	hr20status.valve = hr20statusTemp.valve;
	hr20status.voltage = hr20statusTemp.voltage;
	hr20status.mode = hr20statusTemp.mode;
	hr20status.error_code = hr20statusTemp.error_code;
	hr20status.window_open = hr20statusTemp.window_open;

	hr20status.data_timestamp = uptime;
	hr20status.data_valid = 1;
//    for(i=0;i<4;i++)
//    {
//        if(hr20status->auto_temperature[i] < 500 || hr20status->auto_temperature[i] > 3000)
//            return 0;
//    }
    return 1;
}

void hr20_init(uint8_t set_active)
{
    hr20_active = set_active;

    if(hr20_active)
    {
        uart_init(UART_BAUD_SELECT(UART_BAUDRATE, F_CPU));
		hr20status.data_valid = 0;
		hr20_request_status();
    }
}
// 0         1         2         3         4         5         6         7         8         9
// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
// D: d5 01.01.10 12:07:33 - V: 30 I: 1964 S: 0500 B: 2858 Is: 00000000 Ib: 05 Ic: 28 Ie: 1e X W
// D: d5 01.01.10 15:49:36 - V: 43 I: 1812 S: 1750 B: 2910 Is: 00000000 Ib: 06 Ic: 28 Ie: 1e E:04 X
void hr20_work()
{
	static uint8_t recv_counter = 0;
	char rxbyte;
    static char buffer[150];
	uint8_t win_flag_pos;

    if(!hr20_active)
        return;
	if(!uart_data())
		return;
	
	while(uart_data())
	{
		rxbyte=uart_getchar();
		if(rxbyte == '\n')
		{
			if(buffer[0] == 'D' && recv_counter > 89)
			{
				if((buffer[24] == '-') || (buffer[24] == 'A')) // mode is auto
					hr20statusTemp.mode = 0;
				if(buffer[24] == 'M') // mode is manu
					hr20statusTemp.mode = 1;
				hr20statusTemp.valve = 10*(buffer[29]-48)+(buffer[30]-48);
				hr20statusTemp.tempis = (buffer[38]-48)+
										(buffer[37]-48)*10 +
										(buffer[36]-48)*100 +
										(buffer[35]-48)*1000;
				hr20statusTemp.tempset = (buffer[46]-48)+
										(buffer[45]-48)*10 +
										(buffer[44]-48)*100 +
										(buffer[43]-48)*1000;
				hr20statusTemp.voltage = (buffer[54]-48)+
										(buffer[53]-48)*10 +
										(buffer[52]-48)*100 +
										(buffer[51]-48)*1000;
				if(recv_counter > 90 && buffer[90] == 'E')
				{
					hr20statusTemp.error_code = hexCharToInt(buffer[92])*16 + hexCharToInt(buffer[93]);
					win_flag_pos = 95;
				}
				else
				{
					hr20statusTemp.error_code = 0;
					win_flag_pos = 90;
				}
				if((recv_counter > (win_flag_pos+2) && buffer[win_flag_pos+2] == 'W') 
					|| (recv_counter > win_flag_pos && buffer[win_flag_pos] == 'W'))
				{
					hr20statusTemp.window_open = 1;
				}
				else
				{
					hr20statusTemp.window_open = 0;
				}

				hr20checkPlausibility();
			}
			//else if(buffer[0] == 'R') /* R[ab]=cddd */
			//{
			//	hr20status.last_timer.day = buffer[2]-'0';
			//	hr20status.last_timer.slot = buffer[3]-'0';
			//	hr20status.last_timer.mode = buffer[6]-'0';
			//	hr20status.last_timer.time = buffer[9]-'0' +
			//								 (buffer[8]-'0')*10 +
			//								 (buffer[7]-'0')*100;
			//	can_status_hr20_timer();
			//}
			recv_counter = 0;
		}
		else
		{
			if(recv_counter < 140)
			{
				buffer[recv_counter] = rxbyte;
			}
			recv_counter++;
		}
	}
}

void hr20_request_status(void)
{
    if(!hr20_active)
        return;
	uart_puts("D\n");
}

void parse_hr20_status(char *line, struct _hr20status *hr20status_temp)
{
    /* example line as we receive it:
      D: d6 10.01.09 22:19:14 M V: 54 I: 1975 S: 2000 B: 3171 Is: 00b9 X
     */
}

void parse_hr20_auto_temperature(char *line)
{
    int num;
    /* example line as we receive it:
       G[13]=2d
     */
    num = hexCharToInt(line[2]) * 16 + hexCharToInt(line[3]) - 1;
    hr20status.auto_temperature[num] = (hexCharToInt(line[6])*16 + 
                                              hexCharToInt(line[7]))*50;
}

void hr20SetTemperature(uint8_t temperature)
{
    if(!hr20_active)
        return;
	uart_putc('A');
	uart_putc_hex(temperature);
	uart_putc('\r');
	uart_putc('\n');
}

void hr20SetTime(uint8_t hours, uint8_t mins, uint8_t secs)
{
	if(!hr20_active)
        return;
	uart_putc('H');
	uart_putc_hex(hours);
	uart_putc_hex(mins);
	uart_putc_hex(secs);
	uart_putc('\r');
	uart_putc('\n');
}

void hr20SetDate(uint8_t year, uint8_t month, uint8_t day)
{
	if(!hr20_active)
        return;
	uart_putc('Y');
	uart_putc_hex(year);
	uart_putc_hex(month);
	uart_putc_hex(day);
	uart_putc('\r');
	uart_putc('\n');
}

void hr20GetTimer(uint8_t day, uint8_t slot)
{
//	if(day > 7)
//		return;
//	if(slot>9)
//		return;
//	uart_putc('R');
//	uart_putc(day+'0');
//	uart_putc(slot+'0');
//	uart_putc('\r');
//	uart_putc('\n');
}

void hr20SetTimer(uint8_t day, uint8_t slot, uint8_t mode, uint16_t time)
{
//	uart_putc('W');
//	uart_putc(day+'0');
//	uart_putc(slot+'0');
//	uart_putc(mode+'0');
//	uart_putc_hex_XXX(time);
//	uart_putc('\r');
//	uart_putc('\n');
}

void hr20SetModeManu()
{
    if(!hr20_active)
        return;
	uart_puts("M00\n");
}

void hr20SetModeAuto()
{
    if(!hr20_active)
        return;
	uart_puts("M01\n");
}

static uint8_t hexCharToInt(char c)
{
    if(c <= 57)
        return c - 48;
    return c - 87;
}

