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
#include "config.h"

#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/f1/rcc.h>

#include "dac.h"
#include "vss.h"

#if defined(DAC_BBGAIN_PIN) && defined(DAC_THRLVL_PIN)
#	define HAVE_DAC
#endif

int vss_dac_init(void)
{
#ifdef HAVE_DAC
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, DAC_BBGAIN_PIN);
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
		      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, DAC_THRLVL_PIN);
	rcc_peripheral_enable_clock(&RCC_APB1ENR, RCC_APB1ENR_DACEN);

	dac_disable(CHANNEL_1);
	dac_disable_waveform_generation(CHANNEL_1);
	dac_enable(CHANNEL_1);

	dac_disable(CHANNEL_2);
	dac_disable_waveform_generation(CHANNEL_2);
	dac_enable(CHANNEL_2);

	dac_set_trigger_source(DAC_CR_TSEL1_SW|DAC_CR_TSEL2_SW);
#endif
	return VSS_OK;
}

int vss_dac_set_bbgain(uint8_t gain)
{
#ifdef HAVE_DAC
	dac_load_data_buffer_single(gain, RIGHT8, CHANNEL_1);
	dac_software_trigger(CHANNEL_1);
#endif
	return VSS_OK;
}

int vss_dac_set_trigger(uint16_t thr)
{
#ifdef HAVE_DAC
	dac_load_data_buffer_single(thr, RIGHT12, CHANNEL_2);
	dac_software_trigger(CHANNEL_2);
#endif
	return VSS_OK;
}
