/* Copyright (C) 2016 SensorLab, Jozef Stefan Institute
 * http://sensorlab.ijs.si
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>. */

/* Author: Tomaz Solc, <tomaz.solc@ijs.si> */
#include "base64.h"

#include <assert.h>

static const char trans[] =
	"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define BUFFER_SIZE	8+1
static char buffer[BUFFER_SIZE];

/** @brief Base64-encode a value.
 *
 * n should be set to maximum number of significant bits in x divided by 6. For
 * example, with 12 bits in x, n should be 2.
 *
 * Result is undefined if x < 0.
 *
 * @param x Value to encode.
 * @param n Number of Base64 characters to use.
 * @return Pointer to an internal buffer with encoded string (read-only). */
const char* vss_base64_enc(data_t x, int n)
{
	assert(n < BUFFER_SIZE);

	int i;
	for(i = n-1; i >= 0; i--) {
		buffer[i] = trans[x % 64];
		x /= 64;
	}

	assert(x == 0);

	buffer[n] = 0;

	return buffer;
}
