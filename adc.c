#include <avr/io.h>
#include "main.h"
#include "adc.h"

uint8_t adc_blubb_active;

#define BLUBB_NOISE 10 // 10 * 5mV = 50mV noise +-
#define BLUBB_THRESHOLD 100 // 500mV threshold
#define BLUBB_MIN_TIME 10 // 10 * 4ms = 40ms
#define BLUBB_MAX_TIME 40 // 40 * 4ms = 160ms

uint16_t adc_blubb_value;
uint16_t adc_blubb_normal_pegel;
uint8_t adc_blubb_edge_detected;
uint8_t adc_blubb_time_normal;
uint8_t adc_blubb_time_detected;
uint16_t adc_blubb_last;
uint16_t adc_blubb_times[10];
uint16_t adc_blubb_values[40];
uint8_t adc_blubb_pos;
uint16_t adc_blubb_timer;
uint16_t adc_blubb_time_last;

void adc_blubb_init(uint8_t set_active)
{
	adc_blubb_active = set_active;
	if(!set_active)
		return;

	// set PC5 as input for adc
	DDRC &= ~(1<<PC5);
	PORTC &= ~(1<<PC5);
	
	ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(0<<ADPS0); // prescaler 64 results in 125kHz adc clock (50..200 optimal)
	ADMUX = (1<<REFS0)|(0<<MUX3)|(1<<MUX2)|(0<<MUX1)|(1<<MUX0); //VCC is reference, ADC5 (PC5) is input
	ADCSRA |= 1<<ADSC;

	adc_blubb_times[9] = 0;
}

void adc_blubb_cyclic()
{
	uint16_t adc_temp;
	uint8_t adc_blubb_pos_last;
	uint8_t i;

	if(!adc_blubb_active)
		return;
	while(ADCSRA & (1<<ADSC));
	adc_temp = ADCW;
	adc_blubb_timer++;

//	adc_blubb_normal_pegel = 655;
	if(adc_temp > adc_blubb_normal_pegel)
		adc_blubb_normal_pegel = adc_temp;
//	if(adc_temp < (adc_blubb_normal_pegel - 40))
	if(adc_temp < 140)
	{
		adc_blubb_time_detected++;
	}
	else if(adc_blubb_time_detected > 0)
	{
		if(adc_blubb_time_detected > BLUBB_MIN_TIME &&
			adc_blubb_time_detected < BLUBB_MAX_TIME)
		{

			if(adc_blubb_time_last > adc_blubb_timer) // overflow
			{
				adc_blubb_times[adc_blubb_pos] = 65536 - adc_blubb_time_last + adc_blubb_timer;
			}
			else
			{
				adc_blubb_times[adc_blubb_pos] = adc_blubb_timer - adc_blubb_time_last;
			}
			if(++adc_blubb_pos>9)
				adc_blubb_pos = 0;
			adc_blubb_time_last = adc_blubb_timer;
			adc_blubb_value++;
		}
		adc_blubb_time_detected = 0;
	}

	ADCSRA |= 1<<ADSC;
}

uint8_t getBatteryVoltage()
{
	uint16_t avg = 0;
	uint16_t adc_factor;
	uint8_t i;

	if(adc_blubb_active)
	{
//		return adc_blubb_value;
		for(i=0;i<10;i++)
		{
			avg += adc_blubb_times[i];
		}
		//uptime = avg/10;
		uptime = adc_blubb_value;
		return (avg/10)>>2;
	}
	else
	{
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
}
