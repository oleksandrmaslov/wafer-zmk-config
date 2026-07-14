/*
 * Copyright (c) 2025 Oleksandr Maslov
 *
 * SPDX-License-Identifier: MIT
 *
 * Direct VBAT read from nPM1300 via I2C, wrapped as a Zephyr sensor
 * driver so ZMK can consume it through the standard
 * CONFIG_ZMK_BATTERY_REPORTING_FETCH_MODE_LITHIUM_VOLTAGE fetch path.
 *
 * Registers from the datasheet (ADC bank 0x05):
 *  - TASKVBATMEASURE    offset 0x00: trigger VBAT conversion
 *  - ADCVBATRESULTMSB   offset 0x11: bits [9:2] of the 10-bit result
 *  - ADCGP0RESULTLSBS   offset 0x15: bits [1:0] of the 10-bit result
 */

#define DT_DRV_COMPAT zmk_npm1300_vbat

#include <errno.h>
#include <stdbool.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(npm1300_vbat, CONFIG_SENSOR_LOG_LEVEL);

#define NPM1300_NODE DT_NODELABEL(pmic)

#if !DT_NODE_HAS_STATUS(NPM1300_NODE, okay)
#error "nPM1300 pmic node not found or not okay"
#endif

static const struct i2c_dt_spec npm1300_i2c = I2C_DT_SPEC_GET(NPM1300_NODE);

#define NPM1300_ADC_BANK 0x05

#define NPM1300_REG_TASKVBATMEASURE 0x00
#define NPM1300_REG_ADCVBATRESULTMSB 0x11
#define NPM1300_REG_ADCGP0RESULTLSBS 0x15

#define NPM1300_VBAT_FULL_SCALE_MV 5000
#define NPM1300_VBAT_ADC_STEPS 1023 /* 10-bit ADC: 2^10 - 1 */

int zmk_npm1300_read_vbat_mv(int *out_mv) {
    if (!device_is_ready(npm1300_i2c.bus)) {
        LOG_ERR("nPM1300 I2C bus not ready");
        return -ENODEV;
    }

    uint8_t tx[3] = {NPM1300_ADC_BANK, NPM1300_REG_TASKVBATMEASURE, 0x01};
    int ret = i2c_write_dt(&npm1300_i2c, tx, sizeof(tx));
    if (ret != 0) {
        LOG_ERR("Failed to trigger VBAT measurement: %d", ret);
        return ret;
    }

    /* tCONV is about 250 us; keep a small margin. */
    k_busy_wait(500);

    uint8_t addr[2] = {NPM1300_ADC_BANK, NPM1300_REG_ADCVBATRESULTMSB};
    uint8_t msb;
    ret = i2c_write_read_dt(&npm1300_i2c, addr, sizeof(addr), &msb, 1);
    if (ret != 0) {
        LOG_ERR("Failed to read VBAT MSB: %d", ret);
        return ret;
    }

    addr[1] = NPM1300_REG_ADCGP0RESULTLSBS;
    uint8_t lsb;
    ret = i2c_write_read_dt(&npm1300_i2c, addr, sizeof(addr), &lsb, 1);
    if (ret != 0) {
        LOG_ERR("Failed to read VBAT LSB: %d", ret);
        return ret;
    }

    uint16_t vbat_adc = ((uint16_t)msb << 2) | (lsb & 0x03);
    int32_t mv = (int32_t)vbat_adc * NPM1300_VBAT_FULL_SCALE_MV / NPM1300_VBAT_ADC_STEPS;

    if (mv < 0) {
        mv = 0;
    }

    if (out_mv != NULL) {
        *out_mv = (int)mv;
    }

    LOG_DBG("nPM1300 VBAT: ADC=%u -> %d mV", vbat_adc, (int)mv);
    return 0;
}

/* ------- Zephyr sensor driver wrapper ------- */

struct npm1300_vbat_data {
    int vbat_mv;
};

static bool npm1300_vbat_channel_supported(enum sensor_channel chan)
{
    return chan == SENSOR_CHAN_VOLTAGE || chan == SENSOR_CHAN_GAUGE_VOLTAGE;
}

static int npm1300_vbat_sample_fetch(const struct device *dev, enum sensor_channel chan)
{
    struct npm1300_vbat_data *data = dev->data;

    if (chan != SENSOR_CHAN_ALL && !npm1300_vbat_channel_supported(chan)) {
        return -ENOTSUP;
    }

    int mv = 0;
    int ret = zmk_npm1300_read_vbat_mv(&mv);
    if (ret) {
        return ret;
    }
    data->vbat_mv = mv;
    return 0;
}

static int npm1300_vbat_channel_get(const struct device *dev, enum sensor_channel chan,
                                    struct sensor_value *val)
{
    struct npm1300_vbat_data *data = dev->data;

    if (!npm1300_vbat_channel_supported(chan)) {
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

static int npm1300_vbat_init(const struct device *dev) {
    if (!device_is_ready(npm1300_i2c.bus)) {
        LOG_ERR("nPM1300 I2C bus not ready");
        return -ENODEV;
    }
    return 0;
}

#define NPM1300_VBAT_INIT(inst)                                                                 \
    static struct npm1300_vbat_data npm1300_vbat_data_##inst;                                  \
    DEVICE_DT_INST_DEFINE(inst, npm1300_vbat_init, NULL, &npm1300_vbat_data_##inst, NULL,      \
                          POST_KERNEL, CONFIG_SENSOR_INIT_PRIORITY, &npm1300_vbat_api);

DT_INST_FOREACH_STATUS_OKAY(NPM1300_VBAT_INIT)
