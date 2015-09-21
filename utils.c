/*
 * utils.c
 *
 *  Created on: Sep 20, 2015
 *      Author: scott
 */

#include "utils.h"

int64_t horners_method(int64_t x, polynomial_constants_t constants) {
	int64_t result = 0;
	int i;
	for(i = constants.degree; i >= 0; i--)
	{
		result = result * x + constants.constants[i];
	}
	return result;
}
