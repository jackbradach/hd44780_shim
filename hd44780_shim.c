#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "hd44780.h"
#include "hd44780_avr.h"

#define TRACE_PORT PORTB
#define TRACE_BIT PB1
#define TRACE_DDR DDRB


void setup_clocks(void)
{
    /* We're attached to power, so go for 16mHz */

}


void setup_timers(void)
{

}

void setup(void)
{
    setup_clocks();
    setup_timers();
}

int main(void)
{
    static struct hd44780_desc lcd;
    setup();

    // TODO: this should initialize from a static structure; the hardware
    // layout will not change on the fly.
    hd44780_avr_init(&lcd);
    hd44780_init_lcd(&lcd);
                                    //    "____________________"
    hd44780_put_line(&lcd, HD44780_LINE0, "Sweet evil jesus!");
    hd44780_put_line(&lcd, HD44780_LINE1, "These HD47780s are a");
    hd44780_put_line(&lcd, HD44780_LINE2, "pain in the ass to");
    hd44780_put_line(&lcd, HD44780_LINE3, "make go;  Sorted!");


}
