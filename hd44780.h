#ifndef HD44780_H
#define HD44780_H

#include <util/delay.h>

/* Delays used by the HD44780 module */
#define HD44780_INSTRUCTION_WAIT 15
#define HD44780_INIT_WAIT 50
#define HD44780_CLRHOME_WAIT_MS 1.5
#define HD44780_RESET_WAIT_MS 5

/* Timing minimums, in microseconds. */
#define HD44780_TMIN_Tc 1.200F // Enable Cycle Time
#define HD44780_TMIN_Tpw 1.0F // Enable Pulse Width

/* Shims to make it easier to change how we're introducing delays (eg,
 * busy-wait versus timers).
 */
#define hd44780_delay_us(DELAY_US) _delay_us(DELAY_US)
#define hd44780_delay_ms(DELAY_US) _delay_ms(DELAY_US)

/* LCD Controller Instructions */
#define HD44780_CLEAR_DISPLAY   _BV(0)
#define HD44780_RETURN_HOME     _BV(1)
#define HD44780_ENTRY_MODE      _BV(2)
#define HD44780_DISPLAY_CONTROL _BV(3)
#define HD44780_SHIFT           _BV(4)
#define HD44780_FUNCTION_SET    _BV(5)
#define HD44780_SET_CGRAM_ADDR  _BV(6)
#define HD44780_SET_DDRAM_ADDR  _BV(7)

/* Control port bit definitions */
#define HD44780_ENABLE_GPIO _BV(1)
#define HD44780_RS_GPIO _BV(0)

// XXX - 2016/05/16 - jbradach - R/W not implemented to conserve pins!
// #define HD44780_RW_N_GPIO _BV(5)
// #define HD44780_DATA_BUSY _BV(7)

/* Row offsets for a standard HD44780 */
#define HD44780_ROW0_START  0x0
#define HD44780_ROW0_END    0x13
#define HD44780_ROW1_START  0x40
#define HD44780_ROW1_END    0x53
#define HD44780_ROW2_START  0x14
#define HD44780_ROW2_END    0x27
#define HD44780_ROW3_START  0x54
#define HD44780_ROW3_END    0x67
#define HD44780_ROW_LENGTH  20

enum hd44780_rows {
    HD44780_ROW0 = 0,
    HD44780_ROW1,
    HD44780_ROW2,
    HD44780_ROW3
};

enum {
    HD44780_DIR_WRITE = 0,
    HD44780_DIR_READ
};

/* Prototypes */
struct hd44780_desc {
    int8_t backend;

    /* Pointers to port, PIN, and matching DDR */
    volatile uint8_t *data;
    volatile uint8_t *ctrl;

    uint8_t row_count;
    uint8_t column_count;
};

enum hd44780_backends {
    HD44780_BACKEND_AVR_PORT = 1,
    HD44780_BACKEND_MCP23018,
    HD44780_BACKEND_MCP23S18
};

/* Function prototypes */
void hd44780_init_lcd(struct hd44780_desc *lcd);
void hd44780_goto(struct hd44780_desc *lcd, uint8_t row, uint8_t column);
void hd44780_putc(struct hd44780_desc *lcd, char c);
void hd44780_puts(struct hd44780_desc *lcd, const char *s);
void hd44780_put_line(struct hd44780_desc *lcd, uint8_t line, const char *text);
#endif
