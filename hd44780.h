#ifndef HD44780_H
#define HD44780_H

/* Delays used by the HD44780 module (microseconds) */
#define HD44780_INSTRUCTION_WAIT 15
#define HD44780_CLRHOME_WAIT 1600
#define HD44780_RESET_WAIT 5000
#define HD44780_INIT_WAIT 50

#if defined(CONFIG_HD44780_BACKEND_MCP23018)
#include "mcp23018.h"
#endif

/* Shims to make it easier to change how we're introducing delays (eg,
 * busy-wait versus timers).
 */
#define hd44780_delay_us(DELAY_US) _delay_us(DELAY_US)

/* LCD Controller Instructions */
#define HD44780_CLEAR_DISPLAY   _BV(0)
#define HD44780_RETURN_HOME     _BV(1)
#define HD44780_ENTRY_MODE      _BV(2)
#define HD44780_DISPLAY_CONTROL _BV(3)
#define HD44780_SHIFT           _BV(4)
#define HD44780_FUNCTION_SET    _BV(5)
#define HD44780_SET_CGRAM_ADDR  _BV(6)
#define HD44780_SET_DDRAM_ADDR  _BV(7)

#define HD44780_ENABLE_GPIO _BV(1)
// #define HD44780_RW_N_GPIO _BV(5)
#define HD44780_RS_GPIO _BV(0)

#define HD44780_DATA_BUSY _BV(7)


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
    uint8_t op;
    int8_t backend;
    int8_t dir;

#if defined(CONFIG_HD44780_BACKEND_MCP23018)
    /* MCP23018 specific */
    uint8_t iodir_reg;
    uint8_t io_reg;
    struct mcp23018 mcp23018;
#endif

    /* Pointers to port, PIN, and matching DDR */
    volatile uint8_t *data;
    volatile uint8_t *ctrl;

    uint8_t row;
    uint8_t column;

    uint8_t row_count;
    uint8_t column_count;
};

enum hd44780_backends {
    HD44780_BACKEND_AVR_PORT = 1,
    HD44780_BACKEND_MCP23018
};

/* Timing minimums, in microseconds. */
#define HD44780_TMIN_Tc 1.200F // Enable Cycle Time
#define HD44780_TMIN_Tpw 1.0F // Enable Pulse Width

#if defined(CONFIG_HD44780_BACKEND_MCP23018)
void hd44780_mcp23018_init(struct hd44780_desc *lcd, uint8_t i2c_addr, enum mcp23018_ports port);
#endif

void hd44780_put_line(struct hd44780_desc *lcd, uint8_t line, const char *text);
void hd44780_init_lcd(struct hd44780_desc *lcd);
void hd44780_goto(struct hd44780_desc *lcd, uint8_t row, uint8_t column);
void hd44780_putc(struct hd44780_desc *lcd, char c);
void hd44780_puts(struct hd44780_desc *lcd, const char *s);
#endif
