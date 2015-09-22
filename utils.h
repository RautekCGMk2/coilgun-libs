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
#pragma message("Building for host")
#else // should never be built for something other than host or mcu so this is fine
#define __ARCH_MCU__
#pragma message("Building for mcu")
#endif

#include <stdint.h>

typedef struct {
	int64_t* constants;
	uint8_t degree;
} polynomial_constants_t;

int64_t horners_method(int64_t x, polynomial_constants_t constants);

#endif /* UTILS_H_ */
