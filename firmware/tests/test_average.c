/* Copyright (C) 2013 SensorLab, Jozef Stefan Institute
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
#include "unity.h"
#include "average.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_average_zeros(void)
{
	int size = 100;
	data_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = 0;
	}

	data_t avg = vss_average(buffer, size);

	TEST_ASSERT_EQUAL(0, avg);
}

void test_average_ramp(void)
{
	int size = 101;
	data_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = n;
	}

	data_t avg = vss_average(buffer, size);

	TEST_ASSERT_EQUAL(50, avg);
}

void test_average_overflow(void)
{
	int size = 101;
	data_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = -9000;
	}

	data_t avg = vss_average(buffer, size);

	TEST_ASSERT_EQUAL(-9000, avg);
}

void test_signal_power_zeros(void)
{
	int size = 100;
	uint16_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = 0;
	}

	data_t power = vss_signal_power(buffer, size);

	TEST_ASSERT_EQUAL(INT16_MIN, power);
}

void test_signal_power_constant(void)
{
	int size = 100;
	uint16_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = 10;
	}

	data_t power = vss_signal_power(buffer, size);

	TEST_ASSERT_EQUAL(INT16_MIN, power);
}

void test_signal_power_min(void)
{
	int size = 100;
	uint16_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = n%2;
	}

	data_t power = vss_signal_power(buffer, size);

	TEST_ASSERT_EQUAL(-9633, power);
}


void test_signal_power_max(void)
{
	int size = 100;
	uint16_t buffer[size];
	int n;

	for(n = 0; n < size; n++) {
		buffer[n] = (n%2) * UINT16_MAX;
	}

	data_t power = vss_signal_power(buffer, size);

	TEST_ASSERT_EQUAL(0, power);
}


void test_covariance_dc_max(void)
{
	int size = 25000;
	uint16_t samples[size];
	int l = 20;
	int cov[l];

	int n;
	for(n = 0; n < size; n++) samples[n] = 4095;

	vss_covariance(samples, size, cov, l);

	int l0;
	for(l0 = 0; l0 < l; l0++) {
		TEST_ASSERT_EQUAL(0, cov[l0]);
	}
}

void test_covariance_dc_min(void)
{
	int size = 25000;
	uint16_t samples[size];
	int l = 20;
	int cov[l];

	int n;
	for(n = 0; n < size; n++) samples[n] = 0;

	vss_covariance(samples, size, cov, l);

	int l0;
	for(l0 = 0; l0 < l; l0++) {
		TEST_ASSERT_EQUAL(0, cov[l0]);
	}
}

void test_covariance_power_max(void)
{
	int size = 25000;
	uint16_t samples[size];
	int l = 20;
	int cov[l];

	int n;
	for(n = 0; n < size; n++) samples[n] = 4095 * (n%2);

	vss_covariance(samples, size, cov, l);

	// (4095 - 2047.5)^2 * 8 = 33538050

	int l0;
	for(l0 = 0; l0 < l; l0++) {
		if(l0 % 2 == 0) {
			TEST_ASSERT_EQUAL(33538050, cov[l0]);
		} else {
			TEST_ASSERT_EQUAL(-33538050, cov[l0]);
		}
	}
}

void test_covariance_acc_max(void)
{
	int size = 25000;
	uint16_t samples[size];
	int l = 20;
	int cov[l];

	int n;
	samples[0] = 4095;
	for(n = 1; n < size; n++) samples[n] = 0;

	vss_covariance(samples, size, cov, l);

	TEST_ASSERT_EQUAL(5365, cov[0]);
}
