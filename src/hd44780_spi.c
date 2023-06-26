
#include <stdint.h>
#include <stddef.h>

#include "pico/stdlib.h"
#include "hardware/spi.h"

#include "hd44780_spi.h"

#define bit_read(value, bit) (((value) >> (bit)) & 0x01)
#define bit_set(value, bit) ((value) |= (1UL << (bit)))
#define bit_clear(value, bit) ((value) &= ~(1UL << (bit)))
#define bit_write(value, bit, bitvalue) (bitvalue ? bit_set(value, bit) : bit_clear(value, bit))

// Inspired by https://github.com/matmunk/LiquidCrystal_74HC595

static uint8_t _cols;
static uint8_t _rows;
static uint8_t _display_control;
static uint8_t _display_mode;

static uint8_t _register; // Inhalt Schieberegister

static void transfer() {
    gpio_put(HD44780_SPI_GPIO_CS, 0);
    spi_write_blocking(spi1, &_register, 1);
    gpio_put(HD44780_SPI_GPIO_CS, 1);
}

static void pulse_enable() {
    bit_write(_register, HD44780_SR_BIT_EN, 0);
    transfer();
    sleep_us(1);
    bit_write(_register, HD44780_SR_BIT_EN, 1);
    transfer();
    sleep_us(1);
    bit_write(_register, HD44780_SR_BIT_EN, 0);
    transfer();
    sleep_us(50);
}

static void write_4_bits(uint8_t value) {
    bit_write(_register, HD44780_SR_BIT_D0, (value >> 0) & 1);
    bit_write(_register, HD44780_SR_BIT_D1, (value >> 1) & 1);
    bit_write(_register, HD44780_SR_BIT_D2, (value >> 2) & 1);
    bit_write(_register, HD44780_SR_BIT_D3, (value >> 3) & 1);
    transfer();
    pulse_enable();
}

static void send(uint8_t value, uint8_t mode) {
    bit_write(_register, HD44780_SR_BIT_RS, mode);
    transfer();
    write_4_bits(value >> 4);
    write_4_bits(value);
}

inline static void command(uint8_t value) {
    send(value, 0);
}

size_t lcd_write(uint8_t value) {
    send(value, 1);
    return 1;
}

void lcd_clear() {
    command(LCD_CLEARDISPLAY);
    sleep_us(2000);
}

void lcd_home() {
    command(LCD_RETURNHOME);
    sleep_us(2000);
}

void lcd_no_display() {
    _display_control &= ~LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void lcd_display() {
    _display_control |= LCD_DISPLAYON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void lcd_no_cursor() {
    _display_control &= ~LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void lcd_cursor() {
    _display_control |= LCD_CURSORON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void lcd_no_blink() {
    _display_control &= ~LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void lcd_blink() {
    _display_control |= LCD_BLINKON;
    command(LCD_DISPLAYCONTROL | _display_control);
}

void lcd_scroll_display_left() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}

void lcd_scroll_display_right() {
    command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

void lcd_left_to_right() {
    _display_mode |= LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _display_mode);
}

void lcd_right_to_left() {
    _display_mode &= ~LCD_ENTRYLEFT;
    command(LCD_ENTRYMODESET | _display_mode);
}

void lcd_autoscroll() {
    _display_mode |= LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _display_mode);
}

void lcd_no_autoscroll() {
    _display_mode &= ~LCD_ENTRYSHIFTINCREMENT;
    command(LCD_ENTRYMODESET | _display_mode);
}

void lcd_create_char(uint8_t location, uint8_t charmap[]) {
    location &= 0x7;
    command(LCD_SETCGRAMADDR | (location << 3));
    for (uint8_t i = 0; i < 8; i++) {
        lcd_write(charmap[i]);
    }
}

void lcd_puts(char * str) {
    while(*str != '\0')
        lcd_write(*str++);
}

void lcd_set_cursor(uint8_t col, uint8_t row) {
    uint8_t row_offsets[] = {0x00, 0x40, (uint8_t) 0x00 + _cols, (uint8_t) 0x40 + _cols};
    if (row > _rows) {
        row = _rows - 1;
    }
    command(LCD_SETDDRAMADDR | (col + row_offsets[row]));
}

void lcd_init(uint8_t cols, uint8_t rows, uint8_t charsize) {
    _cols = cols;
    _rows = rows;
    _display_control = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;
    _display_mode = LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;

    // GPIOs initialisieren
    // Initialize CS pin high
    gpio_init(HD44780_SPI_GPIO_CS);
    gpio_set_dir(HD44780_SPI_GPIO_CS, GPIO_OUT);
    gpio_put(HD44780_SPI_GPIO_CS, 1);

    // Initialize SPI port at 8 MHz
    spi_init(spi1, 1000 * 1000);

    // Set SPI format
    spi_set_format( spi1,   // SPI instance
                    8,      // Number of bits per transfer
                    0,      // Polarity (CPOL)
                    0,      // Phase (CPHA)
                    SPI_MSB_FIRST);

    gpio_set_function(HD44780_SPI_GPIO_SCK, GPIO_FUNC_SPI);
    gpio_set_function(HD44780_SPI_GPIO_MOSI, GPIO_FUNC_SPI);

    sleep_us(50000);
    write_4_bits(0x03);
    sleep_us(4500);
    write_4_bits(0x03);
    sleep_us(4500);
    write_4_bits(0x03);
    sleep_us(150);
    write_4_bits(0x02);

    if (_rows > 1) {
        command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS);
    } else if (_rows == 1 && charsize != 0) {
        command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x10DOTS);
    } else {
        command(LCD_FUNCTIONSET | LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS);
    }

    command(LCD_DISPLAYCONTROL | _display_control);
    lcd_clear();
    command(LCD_ENTRYMODESET | _display_mode);
}