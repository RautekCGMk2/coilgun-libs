/*
 * lcd.h
 *
 *  Created on: Sep 21, 2015
 *      Author: scott
 */

#ifndef LCD_H_
#define LCD_H_

#include "utils.h"

#if defined(__ARCH_HOST__)
#include <mraa.h>
#elif defined(__ARCH_MCU__)
#include <mcu_api.h>
#endif

/**
 * Note: The screen is 16x2 so any print functions cannot print more than 32 characters
 * All print functions will truncate a string after the first 32 bytes of data
 * so as to prevent accidental overriding of previous data.
 */

/**
 * Prints a string of up to 32 characters to the screen
 * Any formatting characters such as newline and carradge return will be automatically
 * replaced with the the special cursor move commands before being sent to the device
 * @param string The string to print to the lcd
 */
void lcd_print_raw_string(char* string);

/**
 * Prints a string to a single given line
 */
void lcd_print_line_string(char line, char* string);

/**
 * Prints a line of text up to MAX_SCROLL_LENGTH that will scroll
 * at a rate of 1 character per SCROLL_SPEED ms
 */
void lcd_print_line_scrolling(char line, char* string);

/**
 * Sets the cursor position on the lcd
 * @param line The line number (0 or 1)
 * @param position The zero-indexed position of the cursor (0-15)
 */
void lcd_set_cursor_position(char line, char position);

/**
 * Sets the splash screen that displays on first boot of the lcd
 * @param string The text to display on first boot of the lcd
 */
void lcd_set_spash_screen(char* string);

/**
 * Sets the brightness on the backlight
 * @param brightness A value 0-255 representing 0-100% of screen brightness
 */
void lcd_set_brightness(char brightness);

/**
 * Turn on or off an underline under the cursor position.
 * Useful for debugging purposes
 */
void lcd_cursor_underline_on();
void lcd_cursor_underline_off();

/**
 * Turn on or off blinking the entire character at the cursor position.
 * Useful for debugging purposes
 */
void lcd_cursor_blink_on();
void lcd_curosr_blink_off();


/**
 * Clears the lcd screen
 */
void lcd_clear_screen();

/**
 * Clears a single line on the screen by writing spaces to all characters
 */
void lcd_clear_line(char line);

/**
 * Turns on or off the LCD
 */
void lcd_on();
void lcd_off();


#endif /* LCD_H_ */
