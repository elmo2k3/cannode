#ifndef __ADC_H__
#define __ADC_H__

void adc_blubb_init(uint8_t set_active);
uint8_t getBatteryVoltage(void);
void adc_blubb_cyclic(void);

extern uint16_t adc_blubb_value;

#endif

