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

