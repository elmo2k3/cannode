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
#ifndef __LED_H__
#define __LED_H__

#include <avr/io.h>

#define LED0_ON()	{DDRC |= (1<<KEY0); PORTC &= ~(1<<KEY0);}
#define LED0_OFF()	{DDRC &= ~(1<<KEY0); PORTC |= (1<<KEY0);}

#define LED1_ON()	{DDRC |= (1<<KEY1); PORTC &= ~(1<<KEY1);}
#define LED1_OFF()	{DDRC &= ~(1<<KEY1); PORTC |= (1<<KEY1);}

#define LED2_ON()	{DDRC |= (1<<KEY2); PORTC &= ~(1<<KEY2);}
#define LED2_OFF()	{DDRC &= ~(1<<KEY2); PORTC |= (1<<KEY2);}

#define LED3_ON()	{DDRC |= (1<<KEY3); PORTC &= ~(1<<KEY3);}
#define LED3_OFF()	{DDRC &= ~(1<<KEY3); PORTC |= (1<<KEY3);}

#define LED4_ON()	{DDRC |= (1<<KEY4); PORTC &= ~(1<<KEY4);}
#define LED4_OFF()	{DDRC &= ~(1<<KEY4); PORTC |= (1<<KEY4);}

#define LED5_ON()	{DDRC |= (1<<KEY5); PORTC &= ~(1<<KEY5);}
#define LED5_OFF()	{DDRC &= ~(1<<KEY5); PORTC |= (1<<KEY5);}

#endif

