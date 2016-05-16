#ifndef HD44780_AVR_H
#define HD44780_AVR_H

#include "hd44780.h"

enum hd44780_avr_ports {
    HD44780_AVR_PORT_A,
    HD44780_AVR_PORT_B,
    HD44780_AVR_PORT_C,
    HD44780_AVR_PORT_D,
    HD44780_AVR_PORT_E
};

void hd44780_avr_init(struct hd44780_desc *lcd, volatile uint8_t *port_ctrl, volatile uint8_t *port_data);
void hd44780_avr_write_data(struct hd44780_desc *lcd, uint8_t data);
void hd44780_avr_write_ctrl(struct hd44780_desc *lcd, uint8_t ctrl);
void hd44780_avr_pulse_enable(struct hd44780_desc *lcd);



#endif
