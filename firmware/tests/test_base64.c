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

#include "unity.h"
#include "base64.h"

void setUp(void)
{
}

void tearDown(void)
{
}

void test_one(void)
{
	data_t x = 1;

	const char* r;

	r = vss_base64_enc(x, 1);
	TEST_ASSERT_EQUAL(0, strcmp(r, "B"));

	r = vss_base64_enc(x, 2);
	TEST_ASSERT_EQUAL(0, strcmp(r, "AB"));

	r = vss_base64_enc(x, 3);
	TEST_ASSERT_EQUAL(0, strcmp(r, "AAB"));
}

void test_max(void)
{
	data_t x = 0x0fff;

	const char* r;

	// assertion failure
	//r = vss_base64_enc(x, 1);
	//TEST_ASSERT_EQUAL(0, strcmp(r, "/"));

	r = vss_base64_enc(x, 2);
	TEST_ASSERT_EQUAL(0, strcmp(r, "//"));

	r = vss_base64_enc(x, 3);
	TEST_ASSERT_EQUAL(0, strcmp(r, "A//"));
}
