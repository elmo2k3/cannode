#include <avr/io.h>
#include "main.h"
#include "adc.h"

uint16_t getBatteryVoltage()
{
	uint16_t avg = 0;
	uint16_t adc_factor;
	uint8_t i;

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
	return adc_factor/avg*2;
}
