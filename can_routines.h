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

#endif

