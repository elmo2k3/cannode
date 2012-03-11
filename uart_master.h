#ifndef __UART_MASTER_H__
#define __UART_MASTER_H__


void uart_put_can_msg(can_t *msg);
void uart_master_work(void);

#endif

