/* AVR-specific backend code for controlling the HD44780 LCD interface.
 * This backend is used if you have the LCD hooked directly to the pins
 * of the AVR.  Two sets of ports are needed; one for control and one
 * for data.  I did it that way because the original target was an
 * ATTiny84 and I had to get creative with my I/O usage.
 */
#include <stdio.h>
#include <stdlib.h>
#include <avr/io.h>

#include "hd44780.h"
#include "hd44780_avr.h"

#define HD44780_DATA_MASK 0x0f
#define HD44780_CTRL_MASK 0x03

/* Local prototypes */
static volatile uint8_t *ddr_from_port(volatile uint8_t *port);

/* Initialize the AVR port layer for the hd44780 driver.  This will set
 * the I/O drivers correctly for the pins used and have them driver
 * everything low.  Actual initialization of the LCD controller is done
 * via a higher layer.
 */
void hd44780_avr_init(struct hd44780_desc *lcd, volatile uint8_t *port_ctrl, volatile uint8_t *port_data)
{
    volatile uint8_t *ddr;

    /* Zero the control/data ports before we turn on the drivers.  The LCD
     * controller is slightly fiddly and I really don't want to give it any
     * excuse for being bitchy, like an unexpected pulse putting it intro
     * a goofy state.
     */
    hd44780_avr_write_data(lcd, 0x0);
    hd44780_avr_write_ctrl(lcd, 0x0);

    /* Configure "control" (EN, RS, and R_W# if implemented) port */
    lcd->ctrl = port_ctrl;
    ddr = ddr_from_port(port_ctrl);
    (*ddr) |= HD44780_CTRL_MASK;

    /* Configure 4-bit data port.  Currently hardcoded to be the
     * least-significant 4-bits.  I'll implement something fancier
     * when I actually need it.
     */
    lcd->data = port_data;
    ddr = ddr_from_port(port_data);
    (*ddr) |= HD44780_DATA_MASK;



    lcd->backend = HD44780_BACKEND_AVR_PORT;
}

/* Drive the ctrl value (masked) on the LCD control pins */
void hd44780_avr_write_ctrl(struct hd44780_desc *lcd, uint8_t ctrl)
{
    uint8_t port_ctrl = *lcd->ctrl &= ~HD44780_CTRL_MASK;
    *lcd->ctrl = (port_ctrl | (ctrl & HD44780_CTRL_MASK));
}

/* Drive a 4-bit data value (masked) on the LCD data pins */
void hd44780_avr_write_data(struct hd44780_desc *lcd, uint8_t data)
{
    uint8_t port_data = *lcd->data &= ~HD44780_DATA_MASK;
    *lcd->data = (port_data | (data & HD44780_DATA_MASK));
}

/* Pulse the LCD enable line. */
void hd44780_avr_pulse_enable(struct hd44780_desc *lcd)
{
    *lcd->ctrl &= ~HD44780_ENABLE_GPIO;
    hd44780_delay_us(HD44780_TMIN_Tpw);
    *lcd->ctrl |= HD44780_ENABLE_GPIO;
    hd44780_delay_us(HD44780_TMIN_Tpw);
    *lcd->ctrl &= ~HD44780_ENABLE_GPIO;
}

/* Really simple lookup to get the DDR register from a PORT; one less pointer
 * to have to store in the LCD structure.  The if and #if blocks are structured
 * so that it "should" just work regardless of which ports your device actually
 * implements.
 */
static volatile uint8_t *ddr_from_port(volatile uint8_t *port)
{
#if defined(PORTA)
    if (&PORTA == port)
        return &DDRA;
    else
#endif
#if defined(PORTB)
    if (&PORTB == port)
        return &DDRB;
    else
#endif
#if defined(PORTC)
    if (&PORTC == port)
        return &DDRC;
    else
#endif
#if defined(PORTD)
    if (&PORTD == port)
        return &DDRD;
    else
#endif
#if defined(PORTE)
    if (&PORTE == port)
        return &DDRE;
    else
#endif
        return NULL;
}
