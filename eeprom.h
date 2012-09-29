#ifndef __EEPROM_H__
#define __EEPROM_H__

uint8_t eeprom_get_address(void);
void eeprom_get_relais(uint8_t *addresses, uint8_t *relais);

void eeprom_set_address(uint8_t address);
void eeprom_set_relais(uint8_t key, uint8_t address, uint8_t relais);
void eeprom_set_bandgap(uint8_t set_bandgap);
uint8_t eeprom_get_bandgap(void);
void eeprom_set_uart_master(uint8_t set_uart_master);
uint8_t eeprom_get_uart_master(void);

#endif

