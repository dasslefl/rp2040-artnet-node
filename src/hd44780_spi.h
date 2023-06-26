
// Defines für Display

#ifndef __HD44780_SPI_H__
#define __HD44780_SPI_H__

#define HD44780_SPI_GPIO_MOSI 11
#define HD44780_SPI_GPIO_SCK  10
#define HD44780_SPI_GPIO_CS   13

#define HD44780_SR_BIT_D0   0
#define HD44780_SR_BIT_D1   1
#define HD44780_SR_BIT_D2   2
#define HD44780_SR_BIT_D3   3
#define HD44780_SR_BIT_RS   4
#define HD44780_SR_BIT_EN   5

// taken from https://github.com/matmunk/LiquidCrystal_74HC595

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

size_t lcd_write(uint8_t value);
void lcd_puts(char * str);
void lcd_init(uint8_t cols, uint8_t rows, uint8_t charsize);

void lcd_clear();
void lcd_home();
void lcd_no_display();
void lcd_no_cursor();
void lcd_cursor();
void lcd_no_blink();
void lcd_blink();
void lcd_scroll_display_left();
void lcd_scroll_display_right();
void lcd_left_to_right();
void lcd_right_to_left();
void lcd_autoscroll();
void lcd_no_autoscroll();
void lcd_create_char(uint8_t location, uint8_t charmap[]);
void lcd_set_cursor(uint8_t col, uint8_t row);

#endif