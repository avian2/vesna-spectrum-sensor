/* Copyright (C) 2012 SensorLab, Jozef Stefan Institute
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
#include <stdlib.h>
#include <string.h>

#include "vss.h"
#include "rtc.h"
#include "task.h"
#include "timer.h"

#include "device-dummy.h"

#define CHANNEL_TIME_MS	5

static struct vss_task* current_task = NULL;

typedef int (*data_f)(power_t *val);

static int vss_device_dummy_init(void)
{
	int r;
	
	r = vss_timer_init();
	if(r) return r;

	r = vss_rtc_init();
	if(r) return r;

	return VSS_OK;
}

static int dev_dummy_status(void* priv __attribute__((unused)), char* buffer, size_t len)
{
	const char* status = "dummy device\n";
	if(len < strlen(status) + 1) {
		return VSS_TOO_MANY;
	}

	strncpy(buffer, status, len);
	return VSS_OK;
}

static void do_stuff(void)
{
	power_t result;

	int r;
	r = ((data_f) current_task->sweep_config->device_config->priv)(&result);
	if(r) {
		vss_task_set_error(current_task, "test error");
		current_task = NULL;
		return;
	}

	if(vss_task_insert(current_task, result, vss_rtc_read()) == VSS_OK) {
		vss_timer_schedule(CHANNEL_TIME_MS);
	} else {
		current_task = NULL;
	}
}

void tim4_isr(void)
{
	vss_timer_ack();
	do_stuff();	
}

static int dev_dummy_run(void* priv __attribute__((unused)), struct vss_task* task)
{
	if(current_task != NULL) {
		return VSS_TOO_MANY;
	}

	vss_rtc_reset();

	current_task = task;

	vss_timer_schedule(CHANNEL_TIME_MS);

	return VSS_OK;
}

static const struct vss_device device_dummy = {
	.name = "dummy device",

	.run			= dev_dummy_run,
	.status			= dev_dummy_status,

	.priv 			= NULL
};

static int get_zero(power_t* val)
{
	*val = 0;
	return VSS_OK;
}

static const struct vss_device_config dev_dummy_null_config = {
	.name			= "returns 0 dBm",

	.device			= &device_dummy,

	.channel_base_hz 	= 0,
	.channel_spacing_hz	= 1,
	.channel_bw_hz		= 1,
	.channel_num		= 1000000000,

	.channel_time_ms	= CHANNEL_TIME_MS,

	.priv			= get_zero
};

static int get_random(power_t* val)
{
	*val = -(rand() % 10000);
	return VSS_OK;
}

static const struct vss_device_config dev_dummy_random_config = {
	.name			= "return random power between 0 and -100 dBm",

	.device			= &device_dummy,

	.channel_base_hz 	= 0,
	.channel_spacing_hz	= 1,
	.channel_bw_hz		= 1,
	.channel_num		= 1000000000,

	.channel_time_ms	= CHANNEL_TIME_MS,

	.priv			= get_random
};

static int get_error(power_t* val __attribute__((unused)))
{
	return VSS_ERROR;
}

static const struct vss_device_config dev_dummy_error_config = {
	.name			= "always returns an error",

	.device			= &device_dummy,

	.channel_base_hz 	= 0,
	.channel_spacing_hz	= 1,
	.channel_bw_hz		= 1,
	.channel_num		= 1000000000,

	.channel_time_ms	= CHANNEL_TIME_MS,

	.priv			= get_error
};

int vss_device_dummy_register(void) {
	vss_device_dummy_init();
	vss_device_config_add(&dev_dummy_null_config);
	vss_device_config_add(&dev_dummy_random_config);
	vss_device_config_add(&dev_dummy_error_config);
	return VSS_OK;
}