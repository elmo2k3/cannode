#include <avr/eeprom.h>
#include "eeprom.h"
#include "main.h"

#define ADDRESS_NULL 0xB0
#define RELAIS_ADDRESS_NULL 0xFF
#define RELAIS_RELAIS_NULL 0x00
#define BANDGAP_NULL 123
#define UART_MASTER_NULL 0

uint8_t uart_master_eeprom EEMEM;
uint8_t bandgap_eeprom EEMEM;
uint8_t address_eeprom EEMEM;
uint8_t relais_addresses_eeprom[6] EEMEM;
uint8_t relais_relais_eeprom[6] EEMEM;

uint8_t eeprom_get_address(void)
{
	uint8_t address;
	
	address = eeprom_read_byte(&address_eeprom);

	if(address == 0xFF) // fresh eeprom
		address = ADDRESS_NULL;

	return address;
}

uint8_t eeprom_get_bandgap(void)
{
	uint8_t bandgap;
	
	bandgap = eeprom_read_byte(&bandgap_eeprom);

	if(bandgap == 0xFF) // fresh eeprom
		bandgap = BANDGAP_NULL;

	return bandgap;
}

void eeprom_set_bandgap(uint8_t set_bandgap)
{
	bandgap = set_bandgap;
	eeprom_write_byte(&bandgap_eeprom, set_bandgap);
}

uint8_t eeprom_get_mode(void)
{
	uint8_t uart_master;
	
	uart_master = eeprom_read_byte(&uart_master_eeprom);

	if(uart_master == 0xFF) // fresh eeprom
		uart_master = UART_MASTER_NULL;

	return uart_master;
}

void eeprom_set_uart_master(uint8_t set_uart_master)
{
	eeprom_write_byte(&uart_master_eeprom, set_uart_master);
}

void eeprom_get_relais(uint8_t *addresses, uint8_t *relais)
{
	uint8_t i;

	for(i=0;i<6;i++)
	{
		addresses[i] = eeprom_read_byte(&relais_addresses_eeprom[i]);
		if(addresses[i] == 0xFF) // fresh
			addresses[i] = RELAIS_ADDRESS_NULL;
		relais[i] = eeprom_read_byte(&relais_relais_eeprom[i]);
		if(relais[i] == 0xFF) // fresh
			relais[i] = RELAIS_RELAIS_NULL;
	}
}

void eeprom_set_address(uint8_t set_address)
{
	address = set_address;
	eeprom_write_byte(&address_eeprom, set_address);
}

void eeprom_set_relais(uint8_t key, uint8_t address, uint8_t relais)
{
	relais_addresses[key] = address;
	relais_relais[key] = relais;
	eeprom_write_byte(&relais_addresses_eeprom[key], address);
	eeprom_write_byte(&relais_relais_eeprom[key], relais);
}

