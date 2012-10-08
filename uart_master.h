#ifndef __UART_MASTER_H__
#define __UART_MASTER_H__

#include "can_routines.h"
#include <stdint.h>

void uart_put_can_msg(can_t *msg);
void uart_master_work(void);
void uart_master_init(uint8_t activated);
void uart_putc_hex(uint8_t c);
void uart_putc_hex_XXX(uint16_t c);

#endif

