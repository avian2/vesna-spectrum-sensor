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
#include "vss.h"
#include "device.h"

int vss_device_config_list_num;
const struct vss_device_config* vss_device_config_list[VSS_MAX_DEVICE_CONFIG];

/** @brief Obtain metadata from spectrum sensing device.
 *
 * @param device Pointer to the device to query.
 * @param meta Pointer to structure that will be filled in.
 * @param task Pointer to task that will be started.
 * @return VSS_OK on success or an error code otherwise. */
int vss_device_get_meta(const struct vss_device* device,
		struct vss_data_meta* meta, const struct vss_task* task)
{
	if(device->get_meta != NULL) {
		return device->get_meta(meta, device->priv, task);
	} else {
		return VSS_NOT_SUPPORTED;
	}
}

/** @brief Start a spectrum sensing device.
 *
 * @param device Pointer to the device to start.
 * @param task Pointer to the device task to start.
 * @return VSS_OK on success or an error code otherwise.
 */
int vss_device_run_sweep(const struct vss_device* device, struct vss_task* task)
{
	return device->run(device->priv, task);
}

int vss_device_run_sample(const struct vss_device* device, struct vss_task* task)
{
	if(device->supports_task_baseband) {
		return device->run(device->priv, task);
	} else {
		return VSS_NOT_SUPPORTED;
	}
}

int vss_device_resume(const struct vss_device* device, struct vss_task* task)
{
	if(device->resume == NULL) {
		return VSS_NOT_SUPPORTED;
	}
	return device->resume(device->priv, task);
}

/** @brief Obtain a spectrum sensing device status.
 *
 * @param device Pointer to the device to query.
 * @param buffer Pointer to caller-allocated buffer where the result will be written.
 * @param len Size of the buffer in bytes.
 * @return VSS_OK on success or an error code otherwise.
 */
int vss_device_status(const struct vss_device* device, char* buffer, size_t len)
{
	return device->status(device->priv, buffer, len);
}

/** @brief Obtain the default calibration table.
 *
 * @sa vss_device_get_calibration_t
 *
 * @param device Pointer to the device to query.
 * @param device_config Device configuration that will be used.
 * @return Pointer to calibration table on success or NULL in case of an error.
 */
const struct calibration_point* vss_device_get_calibration(
		const struct vss_device* device,
		const struct vss_device_config* device_config)
{
	if(device->get_calibration != NULL) {
		return device->get_calibration(device->priv, device_config);
	} else {
		return NULL;
	}
}

/** @brief Register a new hardware configuration to the system.
 *
 * @param device_config Pointer to the hardware configuration to register.
 * @return VSS_OK on success or an error code otherwise.
 */
int vss_device_config_add(const struct vss_device_config* device_config)
{
	if(vss_device_config_list_num >= VSS_MAX_DEVICE_CONFIG) return VSS_TOO_MANY;

	vss_device_config_list[vss_device_config_list_num] = device_config;
	vss_device_config_list_num++;

	return VSS_OK;
}

/** @brief Retrieve a registered device configuration.
 *
 * @param device_id Numerical device ID (starting with 0)
 * @param config_id Numerical hardware configuration ID (starting with 0)
 * @return Pointer to device configuration or NULL if configuration was not found.
 */
const struct vss_device_config* vss_device_config_get(int device_id, int config_id)
{
	int device_id_o = -1;
	int config_id_o = 0;

	const struct vss_device* device = NULL;

	int n;
	for(n = 0; n < vss_device_config_list_num; n++) {
		const struct vss_device_config* device_config = vss_device_config_list[n];

		if(device_config->device != device) {
			device = device_config->device;
			device_id_o++;
			config_id_o = 0;
		}

		if(device_id_o == device_id && config_id_o == config_id) {
			return device_config;
		}

		config_id_o++;
	}

	return NULL;
}

/** @brief Return number of channels in a spectrum sweep configuration.
 *
 * @param sweep_config Pointer to a spectrum sweep configuration.
 * @return Number of channels a device will measure using given configuration.
 */
unsigned int vss_sweep_config_channel_num(const struct vss_sweep_config* sweep_config)
{
	return (sweep_config->channel_stop - sweep_config->channel_start - 1)
		/ sweep_config->channel_step + 1;
}
