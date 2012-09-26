#ifndef __HR20_H__
#define __HR20_H__

#include <stdint.h>

/** struct holding complete status information from the hr20 device */
struct _hr20status
{
    int16_t tempis; /**< current temperature */
    int16_t tempset; /**< user set temperature */
    int8_t valve; /**< how open is the valve? in percent */
    int16_t voltage; /**< voltage of the batteries */
    int8_t mode; /**< mode, 1 for manual, 0 for automatic control */
    int16_t auto_temperature[4];
	uint32_t data_timestamp;
	uint8_t error_code;
	uint8_t window_open;
	uint8_t data_valid;
};

extern struct _hr20status hr20status;

void hr20_work(void);
void hr20_init(uint8_t set_active);
void hr20_request_status(void);
void hr20SetTemperature(uint8_t temperature);
void hr20SetModeAuto(void);
void hr20SetModeManu(void);
void hr20SetDate(uint8_t year, uint8_t month, uint8_t day);
void hr20SetTime(uint8_t hours, uint8_t mins, uint8_t secs);

#endif

