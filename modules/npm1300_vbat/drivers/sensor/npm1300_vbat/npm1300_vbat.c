/*
 * Copyright (c) 2025 Oleksandr Maslov
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_npm1300_vbat

#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(npm1300_vbat, CONFIG_SENSOR_LOG_LEVEL);

/* nPM1300 ADC registers (per Nordic nPM1300 PS) */
#define NPM1300_ADC_BASE        0x05U
#define NPM1300_ADC_TASKVBATM   0x00U /* TASKS_VBATMEASURE */
#define NPM1300_ADC_RESULT_MSB  0x11U /* ADCVBATRESULTMSB */
#define NPM1300_ADC_RESULT_LSB  0x12U /* ADCGP0RESULTLSBS, bits[1:0] = VBAT LSBs */

/* VBAT full-scale per datasheet: 5000 mV across 10-bit ADC (1024 codes). */
#define NPM1300_VBAT_FS_MV      5000U
#define NPM1300_VBAT_RES        1024U

struct npm1300_vbat_config {
	const struct device *pmic;
};

struct npm1300_vbat_data {
	uint16_t vbat_mv;
};

static int npm1300_reg_write(const struct device *pmic, uint8_t base, uint8_t off, uint8_t val)
{
	const struct i2c_dt_spec *bus = pmic->config;
	uint8_t buf[3] = { base, off, val };

	return i2c_write_dt(bus, buf, sizeof(buf));
}

static int npm1300_reg_read(const struct device *pmic, uint8_t base, uint8_t off, uint8_t *val)
{
	const struct i2c_dt_spec *bus = pmic->config;
	uint8_t addr[2] = { base, off };

	return i2c_write_read_dt(bus, addr, sizeof(addr), val, 1);
}

static int npm1300_vbat_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
	const struct npm1300_vbat_config *cfg = dev->config;
	struct npm1300_vbat_data *data = dev->data;
	uint8_t msb = 0, lsb = 0;
	int err;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_GAUGE_VOLTAGE) {
		return -ENOTSUP;
	}

	if (!device_is_ready(cfg->pmic)) {
		LOG_ERR("nPM1300 PMIC not ready");
		return -ENODEV;
	}

	err = npm1300_reg_write(cfg->pmic, NPM1300_ADC_BASE, NPM1300_ADC_TASKVBATM, 0x01U);
	if (err) {
		LOG_ERR("VBAT measure trigger failed (%d)", err);
		return err;
	}

	/* Conversion completes well under 1 ms; small delay is plenty. */
	k_msleep(2);

	err = npm1300_reg_read(cfg->pmic, NPM1300_ADC_BASE, NPM1300_ADC_RESULT_MSB, &msb);
	if (err) {
		LOG_ERR("VBAT MSB read failed (%d)", err);
		return err;
	}

	err = npm1300_reg_read(cfg->pmic, NPM1300_ADC_BASE, NPM1300_ADC_RESULT_LSB, &lsb);
	if (err) {
		LOG_ERR("VBAT LSB read failed (%d)", err);
		return err;
	}

	uint16_t code = ((uint16_t)msb << 2) | (lsb & 0x03U);
	uint32_t mv = ((uint32_t)code * NPM1300_VBAT_FS_MV) / NPM1300_VBAT_RES;

	data->vbat_mv = (uint16_t)mv;
	return 0;
}

static int npm1300_vbat_channel_get(const struct device *dev, enum sensor_channel chan,
				    struct sensor_value *val)
{
	struct npm1300_vbat_data *data = dev->data;

	if (chan != SENSOR_CHAN_GAUGE_VOLTAGE) {
		return -ENOTSUP;
	}

	val->val1 = data->vbat_mv / 1000;
	val->val2 = (data->vbat_mv % 1000) * 1000;
	return 0;
}

static const struct sensor_driver_api npm1300_vbat_api = {
	.sample_fetch = npm1300_vbat_sample_fetch,
	.channel_get = npm1300_vbat_channel_get,
};

static int npm1300_vbat_init(const struct device *dev)
{
	const struct npm1300_vbat_config *cfg = dev->config;

	if (!device_is_ready(cfg->pmic)) {
		LOG_ERR("nPM1300 PMIC device not ready");
		return -ENODEV;
	}

	return 0;
}

#define NPM1300_VBAT_INIT(inst)                                                                     \
	static struct npm1300_vbat_data npm1300_vbat_data_##inst;                                   \
	static const struct npm1300_vbat_config npm1300_vbat_cfg_##inst = {                         \
		.pmic = DEVICE_DT_GET(DT_INST_PHANDLE(inst, pmic)),                                 \
	};                                                                                          \
	DEVICE_DT_INST_DEFINE(inst, npm1300_vbat_init, NULL, &npm1300_vbat_data_##inst,             \
			      &npm1300_vbat_cfg_##inst, POST_KERNEL,                                 \
			      CONFIG_SENSOR_INIT_PRIORITY, &npm1300_vbat_api);

DT_INST_FOREACH_STATUS_OKAY(NPM1300_VBAT_INIT)
