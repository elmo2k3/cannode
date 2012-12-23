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
#include "../canlib/can.h"

#ifndef __CAN_ROUTINES_H__
#define __CAN_ROUTINES_H__

void can_parse_msg(can_t *msg);
void can_status_uptime(void);
void can_status_relais(void);
void can_status_powerup(void);
void can_status_relais_eeprom(void);
void can_set_relais(uint8_t address, uint8_t relais, uint8_t state);
void can_status_hr20(void);
void can_status_voltage(void);
void can_status_hr20_timer(void);
void can_routines_send_msg(uint8_t *data, uint8_t length, uint8_t ack_req);

#endif

