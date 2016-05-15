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
    #if 0
                                    //    "____________________"
    hd44780_put_line(&lcd, HD44780_ROW0, "HD47780 shim online!");
    hd44780_put_line(&lcd, HD44780_ROW1, "Yargle Argle");
    hd44780_put_line(&lcd, HD44780_ROW2, "Bargle");
    hd44780_put_line(&lcd, HD44780_ROW3, "Pooooooooooooooooop");
#endif
    while(1) {
    //    uint8_t row = rand() % lcd.row_count;
    //    uint8_t col = rand() % lcd.column_count;

        for (int r = 0; r < 4; r++) {
            for (int c = 0; c < lcd.column_count; c++) {
                hd44780_goto(&lcd, r, c);
                hd44780_putc(&lcd, '*');
                hd44780_goto(&lcd, r, c);
                _delay_ms(150);
                hd44780_putc(&lcd, ' ');
            }
        }

        _delay_ms(100);
    }

}
