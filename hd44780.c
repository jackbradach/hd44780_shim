#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "hd44780.h"
#include "hd44780_avr.h"

enum hd44780_rs_select {
    HD44780_RS_INST = 0,
    HD44780_RS_DATA
};

/* Static prototypes */
static void hd44780_pulse_enable(struct hd44780_desc *lcd);
static void hd44780_reset(struct hd44780_desc *lcd);
static void hd44780_wait_ready(struct hd44780_desc *lcd);
static void hd44780_write_data(struct hd44780_desc *lcd, uint8_t data);
static void hd44780_write_command(struct hd44780_desc *lcd, uint8_t cmd);
static void hd44780_write_command_8bit(struct hd44780_desc *lcd, uint8_t data);

static void hd44780_write_data(struct hd44780_desc *lcd, uint8_t data)
{
    switch(lcd->backend) {
    case HD44780_BACKEND_AVR_PORT:
        hd44780_avr_write_data(lcd, data);
        break;
    default:
        /* NARF! */
        break;
    }
}

static void hd44780_write_ctrl(struct hd44780_desc *lcd, uint8_t ctrl)
{
    switch(lcd->backend) {
    case HD44780_BACKEND_AVR_PORT:
        hd44780_avr_write_ctrl(lcd, ctrl);
        break;
    default:
        /* NARF! */
        break;
    }
}

/* Writes a single byte (of type register or instruction) to the LCD. */
void hd44780_write(struct hd44780_desc *lcd, uint8_t data, uint8_t rs)
{
    uint8_t lcd_ctrl = 0;

    if (rs)
        lcd_ctrl |= HD44780_RS_GPIO;

    hd44780_write_ctrl(lcd, lcd_ctrl);

    /* Upper nibble */
    hd44780_write_data(lcd, (data >> 4));
    hd44780_pulse_enable(lcd);

    /* Lower nibble */
    hd44780_write_data(lcd, (data & 0xf));
    hd44780_pulse_enable(lcd);

    hd44780_write_data(lcd, 0);
}

/* Writes a command (4-bit mode) to the LCD. */
static void hd44780_write_command(struct hd44780_desc *lcd, uint8_t cmd)
{
    hd44780_write(lcd, cmd, 0);
    hd44780_wait_ready(lcd);
}

/* Writes a command (8-bit mode) to the LCD.  This is only used during
 * initialization, prior to kicking over to 4-bit mode.
 */
static void hd44780_write_command_8bit(struct hd44780_desc *lcd, uint8_t data)
{
    hd44780_write_data(lcd, data);
    hd44780_pulse_enable(lcd);
    hd44780_write_data(lcd, 0x0);
}


/* Initialize the HD47780 LCD controller.  This sends the required
 * commands to kick it from 8-bit to 4-bit mode and clear the display.
 */
void hd44780_init_lcd(struct hd44780_desc *lcd)
{
    /* Safety delay to make sure power is up. */
    hd44780_delay_us(HD44780_RESET_WAIT_MS);

    /* Set 4-bit interface */
    hd44780_reset(lcd);

    /* 4x20, no cursor, 5x8 font. */
    hd44780_write_command(lcd, HD44780_DISPLAY_CONTROL);
    hd44780_write_command(lcd, HD44780_CLEAR_DISPLAY);
    hd44780_delay_ms(HD44780_CLRHOME_WAIT_MS);
    hd44780_write_command(lcd, HD44780_ENTRY_MODE | 0x2);
    hd44780_write_command(lcd, HD44780_RETURN_HOME);
    hd44780_delay_us(HD44780_INSTRUCTION_WAIT);
    hd44780_write_command(lcd, HD44780_DISPLAY_CONTROL | 0x4);

    // XXX - hardcoded constants for testing!
    lcd->column_count = 20;
    lcd->row_count = 4;
}

/* Attempt to reset the control interface to the LCD.  It starts up
 * in 8-bit mode, we want 4-bit mode.  The magic it's doing in here
 * works because the message to switch to 4-bit looks the same
 * regardless of how many pins you are using.
 */
static void hd44780_reset(struct hd44780_desc *lcd)
{
    /* We should be in 8-bit mode after reset, but in case we're in
     * an unknown state, flip it first to 8-bit mode and then to
     * 4-bit.
     */
    hd44780_write_command_8bit(lcd, 0x3);
    hd44780_delay_us(HD44780_INIT_WAIT);
    hd44780_write_command_8bit(lcd, 0x3);
    hd44780_delay_us(HD44780_INIT_WAIT);
    hd44780_write_command_8bit(lcd, 0x3);
    hd44780_delay_us(HD44780_INIT_WAIT);
    hd44780_write_command_8bit(lcd, 0x2);
    hd44780_delay_us(HD44780_INIT_WAIT);

    /* After this, we'll be in 4-bit mode. */
    hd44780_write_command(lcd, HD44780_FUNCTION_SET | _BV(3));
}

/* Clear display and set memory pointer to 0x0 */
void hd44780_clr(struct hd44780_desc *lcd)
{
    hd44780_write_command(lcd, HD44780_CLEAR_DISPLAY);
    hd44780_delay_ms(HD44780_CLRHOME_WAIT_MS);
    hd44780_write_command(lcd, HD44780_RETURN_HOME);
    hd44780_delay_us(HD44780_INSTRUCTION_WAIT);
}

/* Pulse the enable signal to the LCD controller, latching
 * the current instruction / data on the falling edge.
 */
static void hd44780_pulse_enable(struct hd44780_desc *lcd)
{
    hd44780_avr_pulse_enable(lcd);
}

/* Put a single character to the LCD */
void hd44780_putc(struct hd44780_desc *lcd, char c)
{
    hd44780_write(lcd, c, HD44780_RS_DATA);
    hd44780_wait_ready(lcd);
}

/* Put a string of characters to the LCD.  No checking is done
 * on bounds!
 */
void hd44780_puts(struct hd44780_desc *lcd, const char *s)
{
    char c;
    while ((c = *s++))
        hd44780_putc(lcd, c);
}

/* Copy a string of text from a buffer to a line on the LCD.  If the length
 * of the string (not including \0 terminator) is longer than the LCD line,
 * it'll be truncated.
 */
void hd44780_put_line(struct hd44780_desc *lcd, uint8_t line, const char *text)
{
    hd44780_goto(lcd, line, 0);

    for (uint8_t i = 0; i < lcd->column_count; i++) {
        /* If we hit a NULL terminator first, we can skip the rest. */
        if ('\0' == *text)
            break;

        hd44780_putc(lcd, *text++);
    }
}

/* Wait for the hd44780 to become ready after an instruction.
 * If the R/W# line were connected, this would read and check
 * the busy bit.  Without it, we simply wait the delay from
 * the datasheet to guarantee that the instruction has completed.
 */
static void hd44780_wait_ready(struct hd44780_desc *lcd)
{
    hd44780_delay_us(HD44780_INSTRUCTION_WAIT);
}

/* Set the LCD's memory pointer (cursor) to a particular row/column. */
void hd44780_goto(struct hd44780_desc *lcd, uint8_t row, uint8_t column)
{
    uint8_t offset;

    /* Wrap the column position in the offset if necessary.  There isn't
     * anything all that sensible to do here when provided an out-of-range
     * column value.  Wrapping should be a hint that this is where to check.
     */
    if (column < lcd->column_count)
        offset = column;
    else
        offset = column % lcd->column_count;

    /* Rows are simply looked up and added to the offset.  Again, there's
     * not a lot to be done if someone passes in row hojillion, so it'll
     * treat any out of range the same as row zero.
     */
    switch (row) {
        default:
        case HD44780_ROW0:
            offset += HD44780_ROW0_START;
            break;
        case HD44780_ROW1:
            offset += HD44780_ROW1_START;
            break;
        case HD44780_ROW2:
            offset += HD44780_ROW2_START;
            break;
        case HD44780_ROW3:
            offset += HD44780_ROW3_START;
            break;
    }

    hd44780_write_command(lcd, HD44780_SET_DDRAM_ADDR | offset);
}
