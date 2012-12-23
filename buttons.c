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
#include <util/delay.h>
#include "buttons.h"

//code taken from http://www.mikrocontroller.net/articles/Entprellung
//
static volatile uint8_t key_state;
static volatile uint8_t key_press;
static volatile uint8_t key_rpt;

uint8_t KEYS[] = {KEY0,KEY1,KEY2,KEY3,KEY4,KEY5};

void buttons_every_10_ms(){
    static uint8_t ct0, ct1, rpt;
    uint8_t i;

    uint8_t port_save;
    uint8_t ddr_save;

    port_save = PORTC;
    ddr_save = DDRC;

    DDRC = 0x00; //all input
    PORTC = 0x3F; // pullups on
    _delay_us(20);

    i = key_state ^ ~(PINC & ((1<<KEY0) | (1<<KEY1) | (1<<KEY2) |
                      (1<<KEY3) | (1<<KEY4) | (1<<KEY5)));
    ct0 = ~(ct0 & i);
    ct1 = ct0 ^(ct1 & i);
    i &= ct0 & ct1;
    key_state ^= i;
    key_press |= key_state & i;

    if((key_state & REPEAT_MASK) == 0){
        rpt = REPEAT_START;
    }
    if(--rpt == 0){
        rpt = REPEAT_NEXT;
        key_rpt |= key_state & REPEAT_MASK;
    }

    PORTC = port_save;
    DDRC = ddr_save;
}

///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed. Each pressed key is reported
// only once
//
uint8_t get_key_press( uint8_t key_mask )
{
  cli();                                          // read and clear atomic !
  key_mask &= key_press;                          // read key(s)
  key_press ^= key_mask;                          // clear key(s)
  sei();
  return key_mask;
}
///////////////////////////////////////////////////////////////////
//
// check if a key has been pressed long enough such that the
// key repeat functionality kicks in. After a small setup delay
// the key is reported beeing pressed in subsequent calls
// to this function. This simulates the user repeatedly
// pressing and releasing the key.
//
uint8_t get_key_rpt( uint8_t key_mask )
{
  cli();                                          // read and clear atomic !
  key_mask &= key_rpt;                            // read key(s)
  key_rpt ^= key_mask;                            // clear key(s)
  sei();
  return key_mask;
}
 
