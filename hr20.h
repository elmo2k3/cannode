#ifndef __HR20_H__
#define __HR20_H__

#include <stdint.h>

void hr20_work(void);
void hr20_init(uint8_t set_active);
void hr20_request_status(void);
void hr20SetTemperature(uint8_t temperature);
void hr20SetModeAuto(void);
void hr20SetModeManu(void);

#endif

