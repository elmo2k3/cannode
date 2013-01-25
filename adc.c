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
#include "main.h"
#include "adc.h"
#include "config.h"

uint8_t adc_blubb_active;

#define BLUBB_NOISE 10 // 10 * 5mV = 50mV noise +-
#define BLUBB_THRESHOLD 100 // 500mV threshold
#define BLUBB_MIN_TIME 10 // 10 * 4ms = 40ms
#define BLUBB_MAX_TIME 40 // 40 * 4ms = 160ms

#ifdef _WITH_BLUBB_COUNTER_
uint16_t adc_blubb_value;
uint8_t adc_blubb_edge_detected;
uint8_t adc_blubb_time_detected;
uint16_t adc_blubb_times[10];
uint16_t adc_blubb_values[20];
uint8_t adc_blubb_pos;
uint8_t adc_blubb_pos_values;
uint16_t adc_blubb_timer;
uint16_t adc_blubb_time_last;

void adc_blubb_init(uint8_t set_active)
{
	adc_blubb_active = set_active;
	if(!set_active)
		return;

	// set PC5 as input for adc
	DDRC &= ~(1<<PC5);
	PORTC &= ~(1<<PC5); // IR
	DDRC |= (1<<PC4); // LED
	PORTC |= (1<<PC4); // LED
	
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0); // prescaler 64 results in 125kHz adc clock (50..200 optimal)
	ADMUX = (1<<REFS0)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0); //VCC is reference, ADC5 (PC5) is input
	ADCSRA |= 1<<ADSC;

	adc_blubb_times[9] = 0;
}

void adc_blubb_cyclic() // every 25ms
{
	uint16_t adc_temp;
	uint16_t adc_blubb_max;
	uint8_t i;
	uint16_t avg; // BEWARE !!! MIGHT OVERFLOW if time between blubs > 160s

	if(!adc_blubb_active)
		return;
	while(ADCSRA & (1<<ADSC));
	adc_temp = ADCW;
	adc_blubb_values[adc_blubb_pos_values] = adc_temp;
	adc_blubb_timer++;

	if(++adc_blubb_pos_values > 19)
		adc_blubb_pos_values = 0;
	
	adc_blubb_max = 0;
	for(i=0;i<19;i++)
	{
		if(adc_blubb_values[i] > adc_blubb_max)
			adc_blubb_max = adc_blubb_values[i];
	}

	if(adc_temp < (adc_blubb_max - 30))
	{
		if(adc_blubb_edge_detected == 0)
		{
			adc_blubb_edge_detected = 1;
			PORTC &= ~(1<<PC4); // LED

			if(adc_blubb_time_last > adc_blubb_timer) // overflow
			{
				adc_blubb_times[adc_blubb_pos] = 65536 - adc_blubb_time_last + adc_blubb_timer;
			}
			else
			{
				adc_blubb_times[adc_blubb_pos] = adc_blubb_timer - adc_blubb_time_last;
			}
			if(adc_blubb_times[adc_blubb_pos] > 3000) // from 75s blubb to blubb do no averaging any more
			{
				adc_blubb_value = adc_blubb_times[adc_blubb_pos];
			}
			else // < 75 do avgerage over last 10 values
			{
				avg = 0;
				for(i=0;i<10;i++)
				{
					avg += adc_blubb_times[i];
				}
				adc_blubb_value = avg/10;
			}
			if(++adc_blubb_pos>9)
				adc_blubb_pos = 0;
			adc_blubb_time_last = adc_blubb_timer;
		}
	}
	if(adc_blubb_edge_detected == 1)
	{
		if(++adc_blubb_time_detected > 10)
		{
			adc_blubb_time_detected = 0;
			adc_blubb_edge_detected = 0;
			PORTC |= (1<<PC4); // LED
		}
	}

	ADCSRA |= 1<<ADSC;
}
#else // ifdef blubb_counter
void adc_blubb_init(uint8_t set_active)
{
}

void adc_blubb_cyclic()
{
}
#endif


uint8_t getBatteryVoltage()
{
	uint16_t avg = 0;
	uint16_t adc_factor;
	uint8_t i;

	if(adc_blubb_active)
	{
		return 0;
	}

	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
	ADMUX = (1<<REFS0)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1); //VCC is reference, 1,23V is the input
	ADCSRA |= 1<<ADSC;
	while(ADCSRA & (1<<ADSC));

	for(i=0;i<5;i++)
	{
		ADCSRA |= 1<<ADSC;
		while(ADCSRA & (1<<ADSC));
		avg += ADCW;
	}

	ADCSRA &= ~(1<<ADEN); //ADC disable for powersaving
	//return 100761/avg; // 1.23 * 1024 * 8 * 10
	adc_factor = bandgap * 256;
	return (adc_factor/avg)*2;
}
