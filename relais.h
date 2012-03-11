#ifndef __RELAIS_H__
#define __RELAIS_H__

#include <stdint.h>

#define RELAIS_1 PD5
#define RELAIS_2 PD6
#define RELAIS_3 PD7
#define RELAIS_4 PB0

void relais_init(void);
void relais_set(uint8_t relais_num, uint8_t val);
uint8_t relais_get(void );

#endif

