/*
 * utils.h
 *
 *  Created on: Sep 20, 2015
 *      Author: scott
 */

#ifndef UTILS_H_
#define UTILS_H_

#if defined(__i586__)
#define __ARCH_HOST__
#else // should never be built for something other than host or mcu so this is fine
#define __ARCH_MCU__
#endif

// The ## before __VA_ARGS__ means that the preprocessor will consume the comma if no var args are supplied
#define SYSLOG_TAG(_log_level, _msg, ...) syslog(_log_level, "[" TAG "] " _msg, ##__VA_ARGS__)

#include <stdint.h>
#include <syslog.h>

typedef struct {
	int64_t* constants;
	uint8_t degree;
} polynomial_constants_t;

typedef struct {
	void* buffer;
	unsigned int pos;
	unsigned int len;
} scrolling_buffer_t;

#define INC_SCROLL_POSITION(_scrollingBuffer) _scrollingBuffer.pos = (_scrollingBuffer.pos + 1) % _scrollingBuffer.len

// only used on host which has pthread support
#define synchronized(lock) \
for (pthread_mutex_t* i_##lock = &lock; i_##lock; \
     i_##lock = NULL, pthread_mutex_unlock(i_##lock)) \
    for (pthread_mutex_lock(i_##lock); i_##lock; i_##lock = NULL) \

int64_t horners_method(int64_t x, polynomial_constants_t constants);

#endif /* UTILS_H_ */
