#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <util/twi.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "hd44780.h"
#include "hd44780_avr.h"



#if defined(CONFIG_HD44780_BACKEND_MCP23018)
#include "mcp23018.h"
#endif

enum hd44780_rs_select {
    HD44780_RS_INST = 0,
    HD44780_RS_DATA
};

/* Static prototypes */
static void hd44780_pulse_enable(struct hd44780_desc *lcd);
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
#if defined(CONFIG_HD44780_BACKEND_MCP23018)
    /* Write a byte to the LCD through the MCP23018. */
    case HD44780_BACKEND_MCP23018:
        mcp23018_write(&lcd->mcp23018, lcd->iodir_reg, data);
        break;
#endif
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
#if defined(CONFIG_HD44780_BACKEND_MCP23018)
    case HD44780_BACKEND_MCP23018:
        mcp23018_write(&lcd->mcp23018, lcd->iodir_reg, ctrl);
        break;
#endif
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

/* Attempt to reset the control interface to the LCD.  It starts up
 * in 8-bit mode, we want 4-bit mode.  The magic it's doing in here
 * works because the message to switch to 4-bit looks the same
 * regardless of how many pins you are using.
 */
void hd44780_reset(struct hd44780_desc *lcd)
{
    /* Safety delay  */
    hd44780_delay_us(HD44780_RESET_WAIT);

    hd44780_write_command_8bit(lcd, 0x3);
    hd44780_delay_us(HD44780_INIT_WAIT);

    hd44780_write_command_8bit(lcd, 0x3);
    hd44780_delay_us(HD44780_INIT_WAIT);
    hd44780_write_command_8bit(lcd, 0x3);
    hd44780_delay_us(HD44780_INIT_WAIT);
    hd44780_write_command_8bit(lcd, 0x2);
    hd44780_delay_us(HD44780_INIT_WAIT);
    hd44780_write_command(lcd, HD44780_FUNCTION_SET | _BV(3));
}

void hd44780_init_lcd(struct hd44780_desc *lcd)
{
    lcd->dir = -1;
    hd44780_delay_us(HD44780_RESET_WAIT);

    /* Set 4-bit interface */
    hd44780_reset(lcd);

    /* 4x20, no cursor, 5x8 font. */
    hd44780_write_command(lcd, HD44780_DISPLAY_CONTROL);
    hd44780_write_command(lcd, HD44780_CLEAR_DISPLAY);
    hd44780_delay_us(HD44780_CLRHOME_WAIT);
    hd44780_write_command(lcd, HD44780_ENTRY_MODE | 0x2);
    hd44780_write_command(lcd, HD44780_RETURN_HOME);
    hd44780_delay_us(HD44780_CLRHOME_WAIT);
    hd44780_write_command(lcd, HD44780_DISPLAY_CONTROL | 0x4);


    // XXX - hardcoded constants for testing!
    lcd->column_count = 20;
    lcd->row_count = 4;
    lcd->row = 0;
    lcd->column = 0;
}

void hd44780_home(void)
{
}

void hd44780_clr(void)
{
}


void hd44780_putc(struct hd44780_desc *lcd, char c)
{
    hd44780_write(lcd, c, HD44780_RS_DATA);
    hd44780_wait_ready(lcd);
}

void hd44780_puts(struct hd44780_desc *lcd, const char *s)
{
    char c;
    while ((c = *s++))
        hd44780_putc(lcd, c);
}

static void hd44780_pulse_enable(struct hd44780_desc *lcd)
{
    hd44780_avr_pulse_enable(lcd);
}

static void hd44780_wait_ready(struct hd44780_desc *lcd)
{
    hd44780_delay_us(HD44780_INSTRUCTION_WAIT);
}

void hd44780_put_line(struct hd44780_desc *lcd, uint8_t line, const char *text)
{
    uint8_t line_addr;

    switch (line) {
    default:
    case HD44780_ROW0:
        line_addr = HD44780_ROW0_START;
        break;
    case HD44780_ROW1:
        line_addr = HD44780_ROW1_START;
        break;
    case HD44780_ROW2:
        line_addr = HD44780_ROW2_START;
        break;
    case HD44780_ROW3:
        line_addr = HD44780_ROW3_START;
        break;
    }

    hd44780_write_command(lcd, HD44780_SET_DDRAM_ADDR | line_addr);
    hd44780_delay_us(HD44780_CLRHOME_WAIT);

    for (uint8_t i = 0; i < lcd->column_count; i++) {
        /* If we hit a NULL terminator first, we can skip the rest. */
        if ('\0' == *text)
            break;

        hd44780_putc(lcd, *text++);
    }
}

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

#if 0
PROCESS(hd44780_test, "hd44780 Test");
PROCESS_THREAD(hd44780_test, ev, data)
{
    PROCESS_BEGIN();

    static struct etimer et;
    static struct hd44780_desc lcd;

    // TODO: this should initialize from a static structure; the hardware
    // layout will not change on the fly.
#if defined(CONFIG_HD44780_BACKEND_MCP23018)
    hd44780_init(&lcd, MCP23018_I2C_BASE_ADDR | (0x7 << 1), MCP23018_PORTA);
#endif
    hd44780_avr_init(&lcd, HD44780_AVR_PORT_B);
    printf("LCD init!\n");
    hd44780_init_lcd(&lcd);
                                    //    "____________________"
    hd44780_put_line(&lcd, HD44780_ROW0, "Sweet evil jesus!");
    hd44780_put_line(&lcd, HD44780_ROW1, "These HD47780s are a");
    hd44780_put_line(&lcd, HD44780_ROW2, "pain in the ass to");
    hd44780_put_line(&lcd, HD44780_ROW3, "make go;  Sorted!");

    printf("done.\n");

    while(1) {

        etimer_set(&et, CLOCK_SECOND);
        PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&et));
    }

    PROCESS_END();
}
#endif
