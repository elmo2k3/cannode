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
#include "relais.h"


void relais_init()
{
	DDRD |= (1<<RELAIS_1) | (1<<RELAIS_2) | (1<<RELAIS_3);
	DDRB |= (1<<RELAIS_4);
}

void relais_set(uint8_t relais_num, uint8_t val)
{
	if(relais_num == RELAIS_4)
	{
		if(val)
			PORTB |= (1<<RELAIS_4);
		else
			PORTB &= ~(1<<RELAIS_4);
	}
	else
	{
		if(val)
			PORTD |= (1<<relais_num);
		else
			PORTD &= ~(1<<relais_num);
	}
}

uint8_t relais_get()
{
	uint8_t retval = 0;

	if(PORTD & (1<<RELAIS_1))
		retval += 1;
	if(PORTD & (1<<RELAIS_2))
		retval += 2;
	if(PORTD & (1<<RELAIS_3))
		retval += 4;
	if(PORTB & (1<<RELAIS_4))
		retval += 8;

	return retval;
}


