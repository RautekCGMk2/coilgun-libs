/*
 * timing.h
 *
 * Contains all the functions and
 *
 *  Created on: Sep 15, 2015
 *      Author: scott
 */

#ifndef TIMING_H_
#define TIMING_H_

#include <stdint.h>

typedef struct {
	int64_t* constants;
	uint8_t numConstants;
} polynomial_constants_t;

typedef struct {
	uint8_t gateNum;
	polynomial_constants_t velocityPoly;
} gate_time_t;

typedef struct {
	uint8_t coilNum;
	uint32_t timingOverride;
	uint8_t gateOverride;
	gate_time_t idealTime;
} timing_data_t;

#endif /* TIMING_H_ */
