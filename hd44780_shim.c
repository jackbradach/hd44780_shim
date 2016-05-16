#include <string.h>
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

/* Control and data ports mapping from the ATTiny to the LCD */
#define HD44780_SHIM_CTRL_PORT PORTB
#define HD44780_SHIM_DATA_PORT PORTA

void hd44780_wipe_horz_right(struct hd44780_desc *lcd, PGM_P const *new_text, uint8_t width, uint8_t frame_delay);
void hd44780_slide_right(struct hd44780_desc *lcd, PGM_P const *old_text, PGM_P const *new_text, uint8_t frame_delay);
//static char get_char_from_progmem(const char **text, uint8_t row, uint8_t col);
extern PGM_P const temp_txt[];
extern PGM_P const nonsense_txt[];

/* Copies the text from a program space string array to the LCD. */
void write_text_from_pgmspace(struct hd44780_desc *lcd, PGM_P const *text, int8_t offset)
{
    for (uint8_t row = 0; row < lcd->row_count; row++) {
        char line[lcd->column_count];

        PGM_P s = pgm_read_word(&text[row]);
        memset(line, ' ', lcd->column_count);

        if (offset >= 0)
            strncpy_P(line + offset, s, lcd->column_count - offset);
        else
            strncpy_P(line, s - offset, lcd->column_count);

        hd44780_put_line(lcd, row, line);
    }
}

/* Micocontroller firmware entry point */
int main(void) __attribute__((OS_main));
int main(void)
{
    static struct hd44780_desc lcd;

    // TODO - 2016/05/16 - jbradach - Write a slightly more elegant
    // initialization routine.  Right now I have to call backend
    // init followed by high level init.  Since the hardware isn't
    // going to change on the fly, I can initialize the LCDs from
    // a static structure (including the backend) and only a single
    // call will be necessary.
    //
    // This is simpler and got the LCD working quickly, however. :)
    hd44780_avr_init(&lcd, &HD44780_SHIM_CTRL_PORT, &HD44780_SHIM_DATA_PORT);
    hd44780_init_lcd(&lcd);

    // XXX - 2016/05/16 - jbradach - Not ready for prime time.
#if 0
    hd44780_slide_right(&lcd, nonsense_txt, temp_txt, 20);
    _delay_ms(1000);
#endif

    /* Demo loop.  */
    while(1) {
        /* Horizontal wipe to display temperature 'card' */
        hd44780_wipe_horz_right(&lcd, temp_txt, 5, 20);
        hd44780_delay_ms(1000);

        /* Horizontal wipe to diplay nonsense text */
        hd44780_wipe_horz_right(&lcd, nonsense_txt, 5, 20);
        hd44780_delay_ms(1000);
    }
}

static char get_char_from_strarray(PGM_P const *text, uint8_t row, uint8_t col)
{
    const char *s = pgm_read_word(&text[row]);
    return pgm_read_byte(&s[col]);
}

/* */
void hd44780_wipe_horz_right(struct hd44780_desc *lcd, PGM_P const *new_text, uint8_t width, uint8_t frame_delay)
{
    for (uint8_t col = 0; col < lcd->column_count + width; col++) {
        if (col < lcd->column_count) {
            for (uint8_t row = 0; row < lcd->row_count; row++) {
                hd44780_goto(lcd, row, col);
                hd44780_putc(lcd, 0xff);
            }
        }

        /* Replace columns outside the width with the new text. */
        if (col >= width) {
            for (uint8_t row = 0; row < lcd->row_count; row++) {
                char c = get_char_from_strarray(new_text, row, col - width);
                hd44780_goto(lcd, row, col - width);
                hd44780_putc(lcd, c);
            }
        }

        for (uint8_t i = 0; i < frame_delay; i++)
            _delay_ms(1);
    }
}

void hd44780_slide_right(struct hd44780_desc *lcd, PGM_P const *old_text, PGM_P const *new_text, uint8_t frame_delay)
{
    /* Slide the "old text" off and replace with the "new text" */
    for (uint8_t col = 0; col <= lcd->column_count; col++) {

        if (col < lcd->column_count) {
            write_text_from_pgmspace(lcd, old_text, col);

            for (uint8_t row = 0; row < lcd->row_count; row++) {
                char c = get_char_from_strarray(old_text, row, col);
                hd44780_goto(lcd, row, col);
                hd44780_putc(lcd, c);
            }
        }

        /* Replace columns outside the width with the new text. */
        if (col > 0) {
            for (uint8_t row = 0; row < lcd->row_count; row++) {
                char c = get_char_from_strarray(new_text, row, col);
                hd44780_goto(lcd, row, col);
                hd44780_putc(lcd, c);
            }
        }

        for (uint8_t i = 0; i < frame_delay; i++)
            _delay_ms(1);
    }
}

const char temp_txt_line0[] PROGMEM = "+---[ Thermals ]---+";
const char temp_txt_line1[] PROGMEM = "| T_ambient:       |";
const char temp_txt_line2[] PROGMEM = "|  Somewhat cold!  |";
const char temp_txt_line3[] PROGMEM = "+-------[02]-------+";

PGM_P const temp_txt[] PROGMEM = {
    temp_txt_line0,
    temp_txt_line1,
    temp_txt_line2,
    temp_txt_line3
};

const char nonsense_txt_line0[] PROGMEM = "+----[Nonsense]----+";
const char nonsense_txt_line1[] PROGMEM = "|   Argle Bargle   |";
const char nonsense_txt_line2[] PROGMEM = "|  Snargle Jargle! |";
const char nonsense_txt_line3[] PROGMEM = "+-------[01]-------+";

PGM_P const nonsense_txt[] PROGMEM = {
    nonsense_txt_line0,
    nonsense_txt_line1,
    nonsense_txt_line2,
    nonsense_txt_line3
};
