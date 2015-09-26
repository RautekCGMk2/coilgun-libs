/*
 * lcd.h
 *
 * Note: The screen is 16x2 so any print functions cannot print more than 32 characters
 * All print functions will truncate a string after the first 32 bytes of data
 * so as to prevent accidental overriding of previous data.
 *
 *  Created on: Sep 21, 2015
 *      Author: scott
 */

#ifndef LCD_H_
#define LCD_H_


#define MAX_SCROLL_LENGTH 	64
#define SCROLL_SPEED		500

/**
 * This must be called before any of the other lcd functions
 * An attempt to call an lcd function before the init will cause an error log message
 * and the function will exit without doing anything
 * @return Returns zero on success or non-zero on failure
 */
int lcd_init();

/**
 * Prints a string of up to 32 characters to the screen
 * Any formatting characters such as newline and carradge return will be automatically
 * replaced with the the special cursor move commands before being sent to the device
 * @param string The string to print to the lcd
 */
int lcd_print_string(char* string);

/**
 * Prints a string to a single given line
 */
int lcd_print_line_string(char line, char* string);

/**
 * Prints a line of text up to MAX_SCROLL_LENGTH that will scroll
 * at a rate of 1 character per SCROLL_SPEED ms
 *
 * Any call to a print function other than this will stop a scrolling message on all lines
 *
 * @param line The line on which to print the scrolling data
 * @param string The string to print. No formatting preprocessing is done
 *
 * @note
 * 		Each line will maintain it's own unique scroll buffer
 * 		Each will scroll at the same time
 *
 * @note
 * 		This functions copies string into an internal buffer, meaning it's safe
 * 		and recommended to free the memory used for string after calling this function
 */
void lcd_print_line_scrolling(char line, char* string);

/**
 * Sets the cursor position on the lcd
 * @param line The line number (0 or 1)
 * @param position The zero-indexed position of the cursor (0-15)
 */
int lcd_set_cursor_position(char line, char position);

/**
 * Sets the splash screen that displays on first boot of the lcd
 * Whatever is currently being displayed is what will be set as the splash screen
 */
int lcd_set_spash_screen();

/**
 * Sets the brightness on the backlight
 * @param brightness A value 0-255 representing 0-100% of screen brightness
 */
int lcd_set_brightness(char brightness);

/**
 * Turn on or off an underline under the cursor position.
 * Useful for debugging purposes
 */
int lcd_cursor_underline_on();
int lcd_cursor_underline_off();

/**
 * Turn on or off blinking the entire character at the cursor position.
 * Useful for debugging purposes
 */
int lcd_cursor_blink_on();
int lcd_curosr_blink_off();


/**
 * Clears the lcd screen
 */
int lcd_clear_screen();

/**
 * Clears a single line on the screen by writing spaces to all characters
 */
int lcd_clear_line(char line);

/**
 * Turns on or off the LCD
 */
int lcd_on();
int lcd_off();


#endif /* LCD_H_ */
