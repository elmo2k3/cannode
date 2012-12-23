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
#ifndef __HR20_H__
#define __HR20_H__

#include <stdint.h>

struct _hr20timer
{
	uint8_t day;
	uint8_t slot;
	uint8_t mode;
	uint16_t time;
};

/** struct holding complete status information from the hr20 device */
struct _hr20status
{
    int16_t tempis; /**< current temperature */
    int16_t tempset; /**< user set temperature */
    int8_t valve; /**< how open is the valve? in percent */
    int16_t voltage; /**< voltage of the batteries */
    int8_t mode; /**< mode, 1 for manual, 0 for automatic control */
    int16_t auto_temperature[4];
	uint32_t data_timestamp;
	uint8_t error_code;
	uint8_t window_open;
	uint8_t data_valid;
	struct _hr20timer last_timer;
};

extern struct _hr20status hr20status;

void hr20_work(void);
void hr20_init(uint8_t set_active);
void hr20_request_status(void);
void hr20SetTemperature(uint8_t temperature);
void hr20SetModeAuto(void);
void hr20SetModeManu(void);
void hr20SetDate(uint8_t year, uint8_t month, uint8_t day);
void hr20SetTime(uint8_t hours, uint8_t mins, uint8_t secs);
void hr20GetTimer(uint8_t day, uint8_t slot);
void hr20SetTimer(uint8_t day, uint8_t slot, uint8_t mode, uint16_t time);

#endif

