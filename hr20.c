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
#include "can_routines.h"

#define UART_BAUDRATE 9600

//#include "hr20.h"

static int16_t hexCharToInt(char c);
static int hr20SerialCommand(char *buffer)
{
}
static uint8_t hr20_active;

/** struct holding complete status information from the hr20 device */
struct _hr20status
{
    int16_t tempis; /**< current temperature */
    int16_t tempset; /**< user set temperature */
    int8_t valve; /**< how open is the valve? in percent */
    int16_t voltage; /**< voltage of the batteries */
    int8_t mode; /**< mode, 1 for manual, 2 for automatic control */
    int16_t auto_temperature[4];
}hr20status;

static int hr20checkPlausibility(struct _hr20status *hr20status)
{
    if(hr20status->mode < 1 || hr20status->mode > 2)
        return 0;
    if(hr20status->tempis < 500 || hr20status->tempis > 4000)
        return 0;
    if(hr20status->tempset < 500 || hr20status->tempset > 3000)
        return 0;
    if(hr20status->valve < 0 || hr20status->valve > 100)
        return 0;
    if(hr20status->voltage < 2000 || hr20status->voltage > 4000)
        return 0;
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
    }
}
// 0        1         2         3         4         5         6         7         8        9
// 01234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890
// D: d5 01.01.10 12:07:33 - V: 30 I: 1964 S: 0500 B: 2858 Is: 00000000 Ib: 05 Ic: 28 Ie: 1e X W
void hr20_work()
{
	static can_t msg;
	static uint8_t recv_counter = 0;
	char rxbyte;
    static char buffer[150];

    if(!hr20_active)
        return;
	if(!uart_data())
		return;
	
    msg.flags.rtr = 0;
	msg.flags.extended = 0;
    msg.id = 0x0F0;
	
    rxbyte=uart_getchar();
	if(rxbyte == '\n')
	{
        msg.length = 8;
        msg.data[0] = MSG_COMMAND_STATUS;
        msg.data[1] = address;
        msg.data[2] = MSG_STATUS_HR20_TEMPIS;
        msg.data[3] = buffer[35];
        msg.data[4] = buffer[36];
        msg.data[5] = buffer[37];
        msg.data[6] = buffer[38];
        msg.data[7] = buffer[92];
        can_send_message(&msg);
		recv_counter = 0;
    }
    else
    {
        if(recv_counter < 150)
            buffer[recv_counter] = rxbyte;
		recv_counter++;
    }
}

void hr20_request_status(void)
{
    uart_puts("D\r");
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

void parse_hr20(char *line)
{
    struct _hr20status hr20status_temp;

    if(strlen(line) < 4)
    {
        return;
    }
    
    if(line[0] == 'D')
    {
        parse_hr20_status(line, &hr20status_temp);
        if(hr20checkPlausibility(&hr20status_temp))
        {
            hr20status.mode = hr20status_temp.mode;
            hr20status.valve = hr20status_temp.valve;
            hr20status.tempis = hr20status_temp.tempis;
            hr20status.tempset = hr20status_temp.tempset;
            hr20status.voltage = hr20status_temp.voltage;
        }

        hr20SerialCommand("G01\r");
        hr20SerialCommand("G02\r");
        hr20SerialCommand("G03\r");
        hr20SerialCommand("G04\r");
    }
    else if(line[0] == 'G')
    {
        parse_hr20_auto_temperature(line);
    }
    else
        return;
    
}

/*!
 ********************************************************************************
 * hr20SetTemperature
 *
 * set the wanted temperature
 *
 * \param temperature the wanted temperature multiplied with 10. only steps
 *      of 5 are allowed
 * \returns returns 1 on success, 0 on failure
 *******************************************************************************/
int hr20SetTemperature(int temperature)
{
    char buffer[16];

    if(temperature % 5) // temperature may only be XX.5°C
        return 1;
    sprintf(buffer,"A%02x\r", temperature/5);
    hr20SerialCommand(buffer);
    return 0;
}

int hr20SetAutoTemperature(int slot, int temperature)
{
    char buffer[16];

    if(temperature % 5) // temperature may only be XX.5°C
        return 0;

    sprintf(buffer,"S%02x%02x\r",slot+1, temperature/5);
    hr20SerialCommand(buffer);
    return 1;
}

/*!
 ********************************************************************************
 * hr20SetModeManu
 *
 * set the mode to manual control
 *******************************************************************************/
void hr20SetModeManu()
{
    hr20SerialCommand("M00\r");
}

/*!
 ********************************************************************************
 * hr20SetModeAuto
 *
 * set the mode to automatic control
 *******************************************************************************/
void hr20SetModeAuto()
{
    hr20SerialCommand("M01\r");
}

//static int16_t hr20GetAutoTemperature(int slot)
//{
//    int i;
//    char buffer[255];
//    char response[255];
//    char *result;
//    
//    sprintf(buffer,"\rG%02x\r",slot+1);
//    
//    for(i=0;i<10;i++)
//    {
//        hr20SerialCommand(buffer, response);
//        if(response[0] == 'G' )
//            break;
//        usleep(1000);
//    }
//    if(response[0] == 'G' && response[5] == '=')
//    {
//        result = strtok(response,"=");
//        result = strtok(NULL,"=");
//    
//        return (hexCharToInt(result[0])*16 + hexCharToInt(result[1]))*50;
//    }
//    else
//        return 0;
//}

static int16_t hexCharToInt(char c)
{
    if(c <= 57)
        return c - 48;
    return c - 87;
}

//float hr20GetTemperatureIs()
//{
//    return hr20status.tempis / 100.0;
//}
//
//float hr20GetAutoTemperature(int slot)
//{
//    if(slot < 0 || slot > 3)
//        return 0.0;
//    return hr20status.auto_temperature[slot] / 100.0;
//}
//
//float hr20GetTemperatureSet()
//{
//    return hr20status.tempset / 100.0;
//}
//
//float hr20GetVoltage()
//{
//    return hr20status.voltage / 1000.0;
//}

//int   hr20GetValve()
//{
//    return hr20status.valve;
//}
//
//int   hr20GetMode()
//{
//    return hr20status.mode;
//}


