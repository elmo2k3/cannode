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

