/*
 * pins.h
 *
 *  Created on: Sep 20, 2015
 *      Author: scott
 */

#ifndef PINS_H_
#define PINS_H_

#include "utils.h"


// TODO Do I want to have only pins numbers here? Or mraa_contexts for the host?
/*
#if defined(__ARCH_HOST__)
#include <mraa.h>
#elif defined(__ARCH_MCU__)
#include <mcu_api.h>
#endif
*/

#define NUM_STAGES		8

//FIXME Add real numbers to these
#define PIN_LCD_UART	 1


#endif /* PINS_H_ */
