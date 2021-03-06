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

/** @file
 */
#ifndef HAVE_AVERAGE_H
#define HAVE_AVERAGE_H

#include "buffer.h"

/** @brief Scaling factor used for fixed point in vss_covariance() */
#define VSS_AVERAGE_SCALE	8

data_t vss_average(const data_t* buffer, size_t len);
data_t vss_signal_power(const uint16_t* buffer, size_t len);
int vss_covariance(const uint16_t* buffer, size_t len, int* cov, size_t l);

#endif
