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
#ifndef __EEPROM_H__
#define __EEPROM_H__

uint8_t eeprom_get_address(void);
void eeprom_get_relais(uint8_t *addresses, uint8_t *relais);

void eeprom_set_address(uint8_t address);
void eeprom_set_button_relais(uint8_t key, uint8_t relais);
void eeprom_set_button_address(uint8_t key, uint8_t address);
void eeprom_set_bandgap(uint8_t set_bandgap);
uint8_t eeprom_get_bandgap(void);
void eeprom_set_uart_master(uint8_t set_uart_master);
uint8_t eeprom_get_mode(void);

#endif

