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
#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <stdint.h>

//code taken from http://www.mikrocontroller.net/articles/Entprellung

#define KEY0    PC2
#define KEY1    PC1
#define KEY2    PC3
#define KEY3    PC0
#define KEY4    PC4
#define KEY5    PC5

#define REPEAT_MASK 0
#define REPEAT_START 50 // 500ms
#define REPEAT_NEXT 20 // 200ms

void buttons_every_10_ms(void);
uint8_t get_key_press(uint8_t key_mask);
uint8_t get_key_rpt(uint8_t key_mask);

extern uint8_t KEYS[];

#endif

