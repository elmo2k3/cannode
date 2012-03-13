#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <string.h>
#include <stdio.h>
#include "../canlib/can.h"
#include "protocol.h"
#include "buttons.h"
#include "relais.h"
#include "led.h"
#include "uart_master.h"
#include "eeprom.h"
#include "can_routines.h"

#define FLAG_125MS 1
#define FLAG_1S 2

#define DEBOUNCE 256

uint8_t address;
volatile uint8_t refreshFlags;
volatile uint32_t uptime;
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
void main()
{
	can_t msg_rx;
	uint8_t i;

	LED0_OFF();
	LED1_OFF();
	LED2_OFF();
	LED3_OFF();
	LED4_OFF();
	LED5_OFF();
	
	address = eeprom_get_address();
	eeprom_get_relais(relais_addresses, relais_relais);

	uart_master_init(0);
	hr20_init(1);

    can_init(BITRATE_125_KBPS);
	can_static_filter(can_filter);

	timer_init();
	relais_init();
	
	can_status_powerup();
	
	sei();
	
    while(1)
    {
		uart_master_work();
		hr20_work();
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

		if(can_check_message() && can_get_message(&msg_rx))
		{
			can_parse_msg(&msg_rx);
			uart_put_can_msg(&msg_rx);
		}
		if(refreshFlags & (1<<FLAG_125MS))
		{
			can_status_relais();
			refreshFlags &= ~(1<<FLAG_125MS);
		}
		if(refreshFlags & (1<<FLAG_1S))
		{
			can_status_uptime();
			refreshFlags &= ~(1<<FLAG_1S);
		}
    }
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

ISR(SIG_OUTPUT_COMPARE1A) {
	static uint8_t prescaler_1s = (uint8_t)DEBOUNCE;
	static uint8_t prescaler_125ms = (uint8_t)(DEBOUNCE/8);
#if F_CPU % DEBOUNCE                     // bei rest
    OCR1A = F_CPU / DEBOUNCE - 1;      // compare DEBOUNCE - 1 times
#endif

	buttons_every_10_ms();
    if( --prescaler_1s == 0 )// every second
	{
        prescaler_1s = (uint8_t)DEBOUNCE;
#if F_CPU % DEBOUNCE         // handle remainder
        OCR1A = F_CPU / DEBOUNCE + F_CPU % DEBOUNCE - 1; // compare once per second
#endif
		uptime++;
		refreshFlags |= (1<<FLAG_1S);
	}

    if( --prescaler_125ms == 0 )// every 125ms
	{
        prescaler_125ms = (uint8_t)(DEBOUNCE/8);
		refreshFlags |= (1<<FLAG_125MS);
	}
}

