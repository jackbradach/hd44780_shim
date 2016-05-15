#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>
#include <util/delay.h>

#include "hd44780.h"
#include "hd44780_avr.h"

#define HD44780_DATA_MASK 0x0f
#define HD44780_CTRL_MASK 0x03

void write_data(struct hd44780_desc *lcd, uint8_t data)
{

}

void write_ctrl(struct hd44780_desc *lcd, uint8_t ctrl)
{
    *lcd->ctrl &= ~HD44780_CTRL_MASK;
    *lcd->ctrl |= (ctrl & HD44780_CTRL_MASK);
}

/* Initialize the AVR port layer for the hd44780 driver. */
void hd44780_avr_init(struct hd44780_desc *lcd)
{
    lcd->data = &PORTA;
    DDRA |= HD44780_DATA_MASK;

    lcd->ctrl = &PORTB;
    DDRB |= HD44780_CTRL_MASK;

    write_data(lcd, 0);
    write_ctrl(lcd, 0x0);

    lcd->backend = HD44780_BACKEND_AVR_PORT;
}

/* Write the data value to the LCD port */
void hd44780_avr_write_data(struct hd44780_desc *lcd, uint8_t data)
{
    uint8_t port_data = *lcd->data &= ~HD44780_DATA_MASK;
    *lcd->data = (port_data | (data & HD44780_DATA_MASK));
}

void hd44780_avr_write_ctrl(struct hd44780_desc *lcd, uint8_t ctrl)
{
    uint8_t port_ctrl = *lcd->ctrl &= ~HD44780_CTRL_MASK;
    *lcd->ctrl = (port_ctrl | (ctrl & HD44780_CTRL_MASK));
}

/* Read control / data value from the LCD port */
uint8_t hd44780_avr_read(struct hd44780_desc *lcd)
{
    // No read line hooked up.
    return 0;
    //return *lcd->pin & HD44780_PORT_MASK;
}

/* Pulses the LCD enable line. */
// TODO - jbradach - 2016/05/01 - Tighten up the timing on this,
// only need like 500ns width for the pulse.
void hd44780_avr_pulse_enable(struct hd44780_desc *lcd)
{
    *lcd->ctrl &= ~HD44780_ENABLE_GPIO;
    _delay_us(HD44780_TMIN_Tpw);
    *lcd->ctrl |= HD44780_ENABLE_GPIO;
    _delay_us(HD44780_TMIN_Tpw);
    *lcd->ctrl &= ~HD44780_ENABLE_GPIO;
//    _delay_us(HD44780_TMIN_Tpw);
}
