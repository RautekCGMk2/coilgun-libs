/*
 * lcd.c
 *
 *  Created on: Sep 22, 2015
 *      Author: scott
 */

#include "utils.h"

#if defined(__ARCH_HOST__)
#include <mraa.h>


#define TAG "LCD"

#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>

#include <pthread.h>

#include "lcd.h"
#include "pins.h"

// NOTE: line is zero-indexed
// 0-15 is line "0" 64-79 is line "1" so we can just bit shift the
// zero-indexed line number and OR it with the position on each line
// 0x80 is a "magic number" needed by the lcd controller
#define CURSOR_POSITION(_line, _position) (0x80 | (_line << 6) | _position)

#define SERLCD_COMMAND_BYTE 			0x7C
#define HD44780_COMMAND_BYTE		0xFE

#define COMMAND_SCROLL_LEFT			0x18
#define COMMAND_SET_SPLASH_SCREEN	(1+'j'-'a') // byte value for control j

// all these are the same command byte internally on the LCD with different flags set
// Turning anything on or off is just flags
// see datasheet for HD44780
#define COMMAND_DISPLAY_ON			0b1100
#define COMMAND_DISPLAY_OFF			0b1000
#define COMMAND_UNDERLINE_ON		(COMMAND_DISPLAY_ON | 0b0010)
#define COMMAND_UNDERLINE_OFF		COMMAND_DISPLAY_ON
#define COMMAND_BLINK_ON			(COMMAND_DISPLAY_ON | 0b0001)
#define COMMAND_BLINK_OFF			COMMAND_DISPLAY_ON
#define COMMAND_BRIGHTNESS_MIN		128
#define COMMAND_BRIGHTNESS_MAX		157


#define ADD_MOVE_CURSOR_TO_BUFFER(_buf, _line, _pos) \
	*_buf = HD44780_COMMAND_BYTE; \
	_buf++; \
	*_buf= CURSOR_POSITION(_line, _pos); \
	_buf++;

#define ADD_MOVE_CURSOR_TO_BUFFER_WITH_LEN(_buf, _line, _pos, _lenvar) \
		ADD_MOVE_CURSOR_TO_BUFFER(_buf, _line, _pos) \
		_lenvar += 2;

char isInit=0;

// thread stuff
pthread_t scrollingThread;
pthread_mutex_t mraaMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t bufferMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t scrollBufferCond = PTHREAD_COND_INITIALIZER;
scrolling_buffer_t line0buffer;
scrolling_buffer_t line1buffer;

mraa_uart_context uartContext = NULL;

int lcd_send_raw_data(char* data, size_t len) {

	int bytesWritten;
	synchronized(mraaMutex) {
			bytesWritten = mraa_uart_write(uartContext, data, len);
	}
	if (bytesWritten < 0) {
		SYSLOG_TAG(LOG_ERR, "Failed to write to LCD uart");
		SYSLOG_TAG(LOG_ERR, "Attempted to send message: %.*s",len, data);
		return bytesWritten;
	}

	mraa_result_t result;
	synchronized(mraaMutex) {
		result = mraa_uart_flush(uartContext);
	}
	if (bytesWritten < 0) {
		SYSLOG_TAG(LOG_ERR, "Failed to write to LCD uart. Error code:%d", result);
		return result * -1; // mraa results are always positive.
	}

	return bytesWritten;
}

int lcd_send_command(char commandByte, char dataByte) {
	char* commands = malloc(2);
	commands[0] = commandByte;
	commands[1] = dataByte;
	int result = lcd_send_raw_data(commands, 2);
	free(commands);
	return result;
}

void lcd_scroll_line(scrolling_buffer_t buffer, char line) {
	char* string = (char*) malloc(5); // 2 to scroll, 2 to move curose, 1 for new character
	string[0] = HD44780_COMMAND_BYTE;
	string[1] = COMMAND_SCROLL_LEFT;
	string[2] = HD44780_COMMAND_BYTE;
	string[3] = CURSOR_POSITION(0, 15); // end of line 1
	string[4] = ((char*)buffer.buffer)[buffer.pos];
	lcd_send_raw_data(string, 5);
	free(string);
}

void* lcd_scroll_thread(void* args) {
	scrolling_buffer_t tmp0;
	scrolling_buffer_t tmp1;

	// this thread should never die and always run in the background.
	// It shouldn't use much CPU time since it's either waiting for data to scroll,
	// or waiting on the scroll speed delay, both of which are basically free
	for(;;) {
		synchronized(bufferMutex) {
			// block until we have something to print
			while (line0buffer.buffer == NULL && line1buffer.buffer == NULL) {
				pthread_cond_wait(&scrollBufferCond, &bufferMutex);
			}
			// copy buffer data into our tmp vars so we can release the lock asap
			// also increment the position variable
			if (line0buffer.buffer) {
				tmp0.len = line0buffer.len;
				tmp0.pos = line0buffer.pos;
				tmp0.buffer = malloc(tmp0.len);
				memcpy(tmp0.buffer, line0buffer.buffer, tmp0.len);
				INC_SCROLL_POSITION(line0buffer);
			}
			if (line1buffer.buffer) {
				tmp1.len = line1buffer.len;
				tmp1.pos = line1buffer.pos;
				tmp1.buffer = malloc(tmp1.len);
				memcpy(tmp1.buffer, line1buffer.buffer, tmp1.len);
				INC_SCROLL_POSITION(line1buffer);
			}
		}

		if (tmp0.buffer) {
			lcd_scroll_line(tmp0, 0);
			// cleanup
			free(tmp0.buffer);
			tmp0.buffer = NULL;
		}
		if (tmp1.buffer) {
			lcd_scroll_line(tmp1, 1);
			// cleanup
			free(tmp1.buffer);
			tmp1.buffer = NULL;
		}

		// sleep for SCROLL_SPEED ms
		usleep(1000);
	}

	pthread_exit(NULL);
}

/* No need to use mutexes since other thread shouldn't be running yet*/
int lcd_init() {
	SYSLOG_TAG(LOG_INFO, "Initializing LCD display"); // syslog *should* have been opened in main
	uartContext = mraa_uart_init(PIN_LCD_UART);
	if (uartContext == NULL) {
		syslog(LOG_ERR, "[LCD] Failed to initialize LCD uart connection");
		return -1;
	}
	mraa_result_t mraaResult = mraa_uart_set_baudrate(uartContext, 9600);
	if (mraaResult != MRAA_SUCCESS) {
		SYSLOG_TAG(LOG_ERR, "Failed to set baudrate on uart. Error code:%d", mraaResult);
		return mraaResult;
	}

	// Init all the pthread stuff
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	// we never need to join the threads so create them detatched
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_mutex_init(&mraaMutex, NULL);
	pthread_mutex_init(&bufferMutex, NULL);
	pthread_cond_init(&scrollBufferCond, NULL);

	// Finally, start scrolling thread
	pthread_create(&scrollingThread, &attr, lcd_scroll_thread, NULL);

	// init complete
	isInit = 1;

	return 0;
}

void lcd_stop_scrolling(char lines) {
	synchronized(bufferMutex) {
		if (lines & 1) line0buffer.buffer = NULL;
		if (lines & 2) line1buffer.buffer = NULL;
	}
}

int lcd_set_splash_screen() {
	return lcd_send_command(SERLCD_COMMAND_BYTE, COMMAND_SET_SPLASH_SCREEN);
}

int lcd_print_string(char* string) {
	lcd_stop_scrolling(1 | 2); // lines 0 and 1
	int len = strlen(string) + 2; // Our new string is guaranteed to be 2 larger to reset cursor pos
	char currLine = 0;
	char currPos = 0;

	// Create a new buffer 16 bytes larger than the original
	// This should be plenty for all the commands that will be added instead of
	// the standard ASCII control characters since the LCD only has 32 characters of display
	// Also the 2 bytes to reset cursor to line 0 position 0
	char* newString = (char*) malloc(strlen(string)+16);

	// add the cursor reset code the the string
	*newString = HD44780_COMMAND_BYTE;
	newString++;
	*newString = CURSOR_POSITION(currLine, currPos);
	newString++;

	for (;*string!=0;string++) {
		switch (*string) {
		case '\n': // move cursor to start of the other line, whichever we're currently on
			currLine = !currLine;
			currPos = 0;
			ADD_MOVE_CURSOR_TO_BUFFER_WITH_LEN(newString, currLine, currPos, len);
			break;
		case '\r': // reset cursor to start of line
			currPos = 0;
			ADD_MOVE_CURSOR_TO_BUFFER_WITH_LEN(newString, currLine, currPos, len);
			break;
//		case '\t': // TODO
//			break;
		default:
			if (currPos == 15) { // at the end of the line
				currPos = 0;
				currLine = !currLine;
				ADD_MOVE_CURSOR_TO_BUFFER_WITH_LEN(newString, currLine, currPos, len);
			}
			*newString = *string;
			newString++;
			len++;
			currPos++;
			break;
		}

	}
	// reset newstring pointer back to start
	newString -= len;

	// finally write our new string, including commands, to the display
	int result = lcd_send_raw_data(newString, len);
	free(newString);
	return result;
}

int lcd_print_line_string(char line, char* string) {
	lcd_stop_scrolling(line);
	int len = strlen(string);
	char* newString = (char*) malloc(16 + 2); // max of 16 chars on a line, plus 2 for control characters
	strncpy(newString, string, 16);
	newString += len;
	ADD_MOVE_CURSOR_TO_BUFFER_WITH_LEN(newString, line, 0, len);

	int result = lcd_send_raw_data(newString, len);
	free(newString);
	return result;
}

void lcd_print_line_scrolling(char line, char* string) {
	int len = strlen(string);
	synchronized(bufferMutex) {
		if (line==0) {
			line0buffer.buffer = malloc(len);
			memcpy(line0buffer.buffer, string, len);
			line0buffer.pos = 0;
			line0buffer.len = len;
		} else if (line==1) {
			line1buffer.buffer = malloc(len);
			memcpy(line1buffer.buffer, string, len);
			line1buffer.pos = 0;
			line1buffer.len = len;
		}
	}
}

int lcd_set_curosr_position(char line, char pos) {
	return lcd_send_command(HD44780_COMMAND_BYTE, CURSOR_POSITION(line, pos));
}

int lcd_set_brightness(char brightness) {
	// rescale brightness to range the lcd expects
	brightness /= COMMAND_BRIGHTNESS_MAX - COMMAND_BRIGHTNESS_MIN;
	brightness += COMMAND_BRIGHTNESS_MIN;
	return lcd_send_command(SERLCD_COMMAND_BYTE, brightness);
}

int lcd_cursor_underline_on() {
	return lcd_send_command(HD44780_COMMAND_BYTE, COMMAND_UNDERLINE_ON);
}

int lcd_cursor_underline_off() {
	return lcd_send_command(HD44780_COMMAND_BYTE, COMMAND_UNDERLINE_OFF);
}

int lcd_cursor_blink_on() {
	return lcd_send_command(HD44780_COMMAND_BYTE, COMMAND_BLINK_ON);
}
int lcd_curosr_blink_off() {
	return lcd_send_command(HD44780_COMMAND_BYTE, COMMAND_BLINK_OFF);
}

int lcd_clear_line(char line) {
	char* spaces;
	spaces = (char*) malloc(16+2); // 16 chars/line + 2 to reset cursor
	memset(spaces, ' ', 16+2);
	ADD_MOVE_CURSOR_TO_BUFFER(spaces, line, 0);
	int result = lcd_print_line_string(line, spaces);
	free(spaces);
	return result;
}

int lcd_clear_screen() {
	// just call the clear line function on both lines
	int result;
	result = lcd_clear_line(0);
	if (result)
		return result;

	result = lcd_clear_line(1);
	return result;
}

#elif defined(__ARCH_MCU__)
#pragma message "Warning, LCD will not function on the MCU"
#endif
