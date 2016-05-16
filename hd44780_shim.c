#include <stdlib.h>
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include <avr/pgmspace.h>

#include "hd44780.h"
#include "hd44780_avr.h"

#define TRACE_PORT PORTB
#define TRACE_BIT PB1
#define TRACE_DDR DDRB

void hd44780_wipe_horz_right(struct hd44780_desc *lcd, PGM_P new_text, uint8_t width, uint8_t frame_delay);
//static char get_char_from_progmem(const char **text, uint8_t row, uint8_t col);
extern PGM_P const temp_txt[];
extern PGM_P const nonsense_txt[];

//static char get_char_from_strarray(const char* const *text, uint8_t row, uint8_t col);

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

void write_text_from_pgmspace(struct hd44780_desc *lcd, PGM_P const *text)
{
    for (uint8_t row = 0; row < lcd->row_count; row++) {
        char line[lcd->column_count];
        PGM_P s = pgm_read_word(&text[row]);
        strncpy_P(line, s, lcd->column_count);
        hd44780_put_line(lcd, row, line);
    }
    //return pgm_read_byte(&s[col]);
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


    while(1) {
    //    uint8_t row = rand() % lcd.row_count;
    //    uint8_t col = rand() % lcd.column_count;
#if 0
        hd44780_put_line(&lcd, HD44780_ROW0, "HD47780 shim online!");
        hd44780_put_line(&lcd, HD44780_ROW1, "Yargle Argle");
        hd44780_put_line(&lcd, HD44780_ROW2, "Bargle");
        hd44780_put_line(&lcd, HD44780_ROW3, "Pooooooooooooooooop");
#endif
        write_text_from_pgmspace(&lcd, nonsense_txt);
        _delay_ms(1000);
    //    hd44780_wipe_horz_right(&lcd, (PGM_P) &temp_txt, 10, 20);
    //    _delay_ms(1000);

    }

}


/* */
void hd44780_wipe_horz_right(struct hd44780_desc *lcd, PGM_P new_text, uint8_t width, uint8_t frame_delay)
{
    for (uint8_t col = 0; col < lcd->column_count + width; col++) {
        if (col < lcd->column_count) {
            for (uint8_t row = 0; row < lcd->row_count; row++) {
                hd44780_goto(lcd, row, col);
                hd44780_putc(lcd, 0xff);
            }
        }

        if (col >= width) {
            for (uint8_t row = 0; row < lcd->row_count; row++) {
            //    char c = pgm_read_byte(&temperature_text[row][col - width]);
                //char c = get_char_from_strarray(new_text, row, col - width);
                hd44780_goto(lcd, row, col - width);
            //    hd44780_putc(lcd, c);
            }
        }

        for (uint8_t i = 0; i < frame_delay; i++)
            _delay_ms(1);
    }
}

#if 0
static char get_char_from_strarray(PGM_P const *text, uint8_t row, uint8_t col)
{
    const char *s = pgm_read_word(text[row]);
    return pgm_read_byte(&s[col]);
}


#endif
const char temp_txt_line0[] PROGMEM = "+--------YYY-------+";
const char temp_txt_line1[] PROGMEM = "|   Temperature:   |";
const char temp_txt_line2[] PROGMEM = "|  Is cold, bitch! |";
const char temp_txt_line3[] PROGMEM = "+--------XXX-------+";

PGM_P const temp_txt[] PROGMEM = {
    temp_txt_line0,
    temp_txt_line1,
    temp_txt_line2,
    temp_txt_line3
};

const char nonsense_txt_line0[] PROGMEM = "+-------VVV--------+";
const char nonsense_txt_line1[] PROGMEM = "|   Argle Bargle   |";
const char nonsense_txt_line2[] PROGMEM = "|  Snargle Jargle! |";
const char nonsense_txt_line3[] PROGMEM = "+-------^^^--------+";

PGM_P const nonsense_txt[] PROGMEM = {
    nonsense_txt_line0,
    nonsense_txt_line1,
    nonsense_txt_line2,
    nonsense_txt_line3
};
