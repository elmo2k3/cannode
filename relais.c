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


