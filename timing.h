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

#include "utils.h"
#include "pins.h"

typedef struct {
	uint8_t gateNum;
	polynomial_constants_t velocityPoly;
} gate_time_t;

typedef struct {
	uint8_t stageNum;
	uint32_t timingOverride;
	uint8_t gateOverride;
	gate_time_t idealTime;
} stage_timing_data_t;

typedef struct {
	stage_timing_data_t* data;
} shot_timing_data_t;

typedef struct {
	uint8_t stageNum;
	uint32_t timeFired;
	uint32_t velocity;
} fire_data_t;

#endif /* TIMING_H_ */
