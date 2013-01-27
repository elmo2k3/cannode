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
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>
#include <avr/sleep.h>
#include "../canlib/can.h"
#include "protocol.h"
#include "buttons.h"
#include "relais.h"
#include "led.h"
#include "uart_master.h"
#include "eeprom.h"
#include "can_routines.h"
#include "hr20.h"

#define FLAG_250MS 1
#define FLAG_1S 2

#define DEBOUNCE 256

uint8_t address;
uint8_t bandgap;
volatile uint8_t mode;
volatile uint8_t refreshFlags;
volatile uint32_t uptime;
volatile uint16_t uptime_miliseconds;
uint8_t relais_addresses[6];
uint8_t relais_relais[6];

void timer_init(void);

// every node receives everything
const prog_uint8_t can_filter[] = {
	// Group 0
	MCP2515_FILTER(0),				// Filter 0
	MCP2515_FILTER(0),				// Filter 1
	
	// Group 1
	MCP2515_FILTER(0),				// Filter 2
	MCP2515_FILTER(0),				// Filter 3
	MCP2515_FILTER(0),				// Filter 4
	MCP2515_FILTER(0),				// Filter 5
	
	MCP2515_FILTER(0x0),				// Mask 0 (for group 0)
	MCP2515_FILTER(0x0),				// Mask 1 (for group 1)
};

int main()
{
	can_t msg_rx;
	uint8_t i;
	uint8_t minute_counter;

	LED0_OFF();
	LED1_OFF();
	LED2_OFF();
	LED3_OFF();
	LED4_OFF();
	LED5_OFF();
	
	address = eeprom_get_address();
	eeprom_get_relais(relais_addresses, relais_relais);
	bandgap = eeprom_get_bandgap();
	mode = eeprom_get_mode();

	PORTD = (1<<PD0); // pullup for rxd pin
	DDRD = (1<<PD1); // output for tx pin

	uart_master_init((mode & MODE_UART_MASTER)!=0);
	hr20_init((mode & MODE_HR20)!=0);
	adc_blubb_init((mode & MODE_BLUBB_COUNTER)!=0);

    can_init(BITRATE_125_KBPS);
	can_static_filter(can_filter);

	timer_init();
	relais_init();
	
	can_status_powerup();
	
	sei();
	
	minute_counter = 0;
	set_sleep_mode(SLEEP_MODE_IDLE);
    while(1)
    {
		uart_master_work();
		hr20_work();
		if(!(mode & MODE_BLUBB_COUNTER))
		{
			for(i=0;i<6;i++)
			{
				if(get_key_press(1<<KEYS[i]))
				{
					if(DDRC & (1<<KEYS[i])) // LED is on
						can_set_relais(relais_addresses[i], relais_relais[i],0);
					else
						can_set_relais(relais_addresses[i], relais_relais[i],1);
				}
			}
		}

		if(can_check_message() && can_get_message(&msg_rx))
		{
			can_parse_msg(&msg_rx);
			uart_put_can_msg(&msg_rx);
		}
		if(refreshFlags & (1<<FLAG_250MS))
		{
			can_status_relais();
			refreshFlags &= ~(1<<FLAG_250MS);
		}
		if(refreshFlags & (1<<FLAG_1S))
		{
			can_status_uptime();
			can_status_hr20();
			if(++minute_counter == 60)
			{
				hr20_request_status();
				minute_counter = 0;
			}
			refreshFlags &= ~(1<<FLAG_1S);
		}
		sleep_mode();
    }
	return 0;
}

void timer_init()
{
	/* Prescaler 1024 */
	TCCR0 = (1<<CS02) | (1<<CS00);
    TCCR1B = (1<<WGM12) | (1<<CS10); // clear on compare, dont start yet
    OCR1A = F_CPU / DEBOUNCE -1;
    TCNT1 = 0;
    
    TIMSK = (1<<OCIE1A);
}

ISR(TIMER1_COMPA_vect) { // called every 1/256s = 4ms
	static uint8_t prescaler_1s = (uint8_t)DEBOUNCE;
	static uint8_t prescaler_250ms = (uint8_t)(DEBOUNCE/4);
	static uint8_t prescaler_25ms = (uint8_t)(DEBOUNCE/40);
#if F_CPU % DEBOUNCE                     // bei rest
    OCR1A = F_CPU / DEBOUNCE - 1;      // compare DEBOUNCE - 1 times
#endif

	uptime_miliseconds += 4;
	if(uptime_miliseconds > 0xEA5F) // 60seconds
		uptime_miliseconds = 0;

	if(mode & MODE_BLUBB_COUNTER)
	{
		if(--prescaler_25ms == 0)
		{
			prescaler_25ms = (uint8_t)(DEBOUNCE/40);
			adc_blubb_cyclic();
		}
	}
	else
	{
		buttons_every_10_ms();
	}

    if( --prescaler_1s == 0 )// every second
	{
        prescaler_1s = (uint8_t)DEBOUNCE;
#if F_CPU % DEBOUNCE         // handle remainder
        OCR1A = F_CPU / DEBOUNCE + F_CPU % DEBOUNCE - 1; // compare once per second
#endif
		uptime++;
		refreshFlags |= (1<<FLAG_1S);
	}

    if( --prescaler_250ms == 0 )// every 250ms
	{
        prescaler_250ms = (uint8_t)(DEBOUNCE/4);
		refreshFlags |= (1<<FLAG_250MS);
	}
}

