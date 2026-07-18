/*
 * Copyright (c) 2020 Rohit Gujarathi
 * SPDX-FileCopyrightText: Copyright The Zephyr Project Contributors
 * Copyright (c) 2026 Oleksandr Maslov
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#define DT_DRV_COMPAT wafer_ls0xx_safe

#include <errno.h>
#include <string.h>

#include <zephyr/device.h>
#include <zephyr/drivers/display.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/spi.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/sys/util.h>

LOG_MODULE_REGISTER(wafer_ls0xx, CONFIG_DISPLAY_LOG_LEVEL);

/* Supports LS012B7DD01, LS013B7DH03, LS013B7DH05,
 * LS027B7DH01A, LS032B7DD02, and LS044Q7DH01.
 */

/* A high bit is white, a low bit is black, and the panel expects LSB first. */
#define LS0XX_PANEL_WIDTH DT_INST_PROP(0, width)
#define LS0XX_PANEL_HEIGHT DT_INST_PROP(0, height)

#define LS0XX_PIXELS_PER_BYTE 8U

/* Each row contains its line number, the pixel data, and a dummy byte. */
#define LS0XX_BYTES_PER_LINE ((LS0XX_PANEL_WIDTH / LS0XX_PIXELS_PER_BYTE) + 2)

#define LS0XX_BIT_WRITECMD 0x01
#define LS0XX_BIT_VCOM 0x02
#define LS0XX_BIT_CLEAR 0x04

#define LS0XX_VCOM_PRIO CONFIG_WAFER_LS0XX_VCOM_THREAD_PRIO
#define LS0XX_VCOM_STACK_SIZE CONFIG_WAFER_LS0XX_VCOM_THREAD_STACK_SIZE

/* One dropped 10 fps frame is the maximum useful wait. The short delay before
 * releasing the SPI bus matches the timing used by the pinned Zephyr driver.
 */
#define LS0XX_MAX_BUS_WAIT_MSEC 100
#define LS0XX_BUS_RETURN_DELAY_TICKS 4

struct ls0xx_data {
	bool vcom_state;
};

#if DT_INST_PROP(0, serial_vcom_inversion) || DT_INST_NODE_HAS_PROP(0, extcomin_gpios)
#define USE_VCOM_THREAD true
#endif

struct ls0xx_config {
	struct spi_dt_spec bus;
#if DT_INST_NODE_HAS_PROP(0, disp_en_gpios)
	struct gpio_dt_spec disp_en_gpio;
#endif
#if DT_INST_NODE_HAS_PROP(0, extcomin_gpios)
	struct gpio_dt_spec extcomin_gpio;
#endif
#if DT_INST_PROP(0, serial_vcom_inversion)
	int serial_vcom_int;
#endif
};

/* A mutex provides ownership checking and priority inheritance while display
 * refreshes and serial VCOM commands share the SPI transaction.
 */
K_MUTEX_DEFINE(ls0xx_bus_mutex);

static int ls0xx_blanking_off(const struct device *dev)
{
#if DT_INST_NODE_HAS_PROP(0, disp_en_gpios)
	const struct ls0xx_config *config = dev->config;

	return gpio_pin_set_dt(&config->disp_en_gpio, 1);
#else
	LOG_WRN("Unsupported");
	return -ENOTSUP;
#endif
}

static int ls0xx_blanking_on(const struct device *dev)
{
#if DT_INST_NODE_HAS_PROP(0, disp_en_gpios)
	const struct ls0xx_config *config = dev->config;

	return gpio_pin_set_dt(&config->disp_en_gpio, 0);
#else
	LOG_WRN("Unsupported");
	return -ENOTSUP;
#endif
}

static int ls0xx_cmd(const struct device *dev, uint8_t *buf, size_t len)
{
	const struct ls0xx_config *config = dev->config;
	struct spi_buf cmd_buf = {.buf = buf, .len = len};
	struct spi_buf_set buf_set = {.buffers = &cmd_buf, .count = 1};
	const uint8_t base_command = buf[0];
	int err;

#if DT_INST_PROP(0, serial_vcom_inversion)
	struct ls0xx_data *data = dev->data;

	buf[0] = base_command | (data->vcom_state ? LS0XX_BIT_VCOM : 0);
#endif

	err = spi_write_dt(&config->bus, &buf_set);
	buf[0] = base_command;

	if (err < 0) {
		LOG_ERR("LS0xx command write failed: %d", err);
		return err;
	}

#if DT_INST_PROP(0, serial_vcom_inversion)
	/* Advance the software polarity only after the panel received the command. */
	data->vcom_state = !data->vcom_state;
#endif

	return 0;
}

/* The caller must own ls0xx_bus_mutex and must have attempted an SPI transfer.
 * SPI_LOCK_ON must be released before another thread can acquire the mutex.
 */
static int ls0xx_release_bus(const struct device *dev)
{
	const struct ls0xx_config *config = dev->config;
	int release_err;
	int unlock_err;

	k_sleep(K_TICKS(LS0XX_BUS_RETURN_DELAY_TICKS));
	release_err = spi_release_dt(&config->bus);
	if (release_err < 0) {
		LOG_ERR("Failed to release LS0xx SPI bus: %d", release_err);
	}

	unlock_err = k_mutex_unlock(&ls0xx_bus_mutex);
	if (unlock_err < 0) {
		LOG_ERR("Failed to unlock LS0xx bus mutex: %d", unlock_err);
	}

	return release_err < 0 ? release_err : unlock_err;
}

#ifdef USE_VCOM_THREAD
static void ls0xx_vcom_toggle(void *a, void *b, void *c)
{
	const struct device *dev = a;
	const struct ls0xx_config *config = dev->config;

	ARG_UNUSED(b);
	ARG_UNUSED(c);

	while (true) {
#if DT_INST_NODE_HAS_PROP(0, extcomin_gpios)
		gpio_pin_toggle_dt(&config->extcomin_gpio);
		k_usleep(3);
		gpio_pin_toggle_dt(&config->extcomin_gpio);
		k_msleep(1000 / DT_INST_PROP(0, extcomin_frequency));
#elif DT_INST_PROP(0, serial_vcom_inversion)
		int err = k_mutex_lock(&ls0xx_bus_mutex, K_MSEC(LS0XX_MAX_BUS_WAIT_MSEC));

		if (err == 0) {
			uint8_t empty_cmd[2] = {0, 0};
			int command_err = ls0xx_cmd(dev, empty_cmd, sizeof(empty_cmd));
			int release_err = ls0xx_release_bus(dev);

			if (command_err < 0) {
				LOG_ERR("Failed to toggle display VCOM: %d", command_err);
			} else if (release_err < 0) {
				LOG_ERR("Failed to finish display VCOM transaction: %d", release_err);
			}
		} else {
			/* Do not release either synchronization primitive: this thread does
			 * not own them when the mutex acquisition times out.
			 */
			LOG_ERR("LS0xx bus mutex unavailable for VCOM: %d", err);
		}

		k_msleep(config->serial_vcom_int);
#endif
	}
}

K_THREAD_STACK_DEFINE(vcom_toggle_stack, LS0XX_VCOM_STACK_SIZE);
static struct k_thread vcom_toggle_thread;
#endif

static int ls0xx_clear(const struct device *dev)
{
	uint8_t clear_cmd[2] = {LS0XX_BIT_CLEAR, 0};
	int err = k_mutex_lock(&ls0xx_bus_mutex, K_MSEC(LS0XX_MAX_BUS_WAIT_MSEC));
	int release_err;

	if (err < 0) {
		LOG_ERR("LS0xx bus mutex unavailable for clear: %d", err);
		return err;
	}

	err = ls0xx_cmd(dev, clear_cmd, sizeof(clear_cmd));
	release_err = ls0xx_release_bus(dev);

	return err < 0 ? err : release_err;
}

static int ls0xx_update_display(const struct device *dev, uint16_t start_line,
				uint16_t num_lines, const uint8_t *data)
{
	const struct ls0xx_config *config = dev->config;
	uint8_t write_cmd[1] = {LS0XX_BIT_WRITECMD};
	uint8_t trailing_dummy = 0;
	uint8_t line_number = 0;
	uint8_t dummy = 27;
	struct spi_buf trailing_buf = {
		.buf = &trailing_dummy,
		.len = sizeof(trailing_dummy),
	};
	struct spi_buf_set trailing_set = {
		.buffers = &trailing_buf,
		.count = 1,
	};
	struct spi_buf line_buf[3] = {
		{
			.len = sizeof(line_number),
			.buf = &line_number,
		},
		{
			.len = LS0XX_BYTES_PER_LINE - 2,
		},
		{
			.len = sizeof(dummy),
			.buf = &dummy,
		},
	};
	struct spi_buf_set line_set = {
		.buffers = line_buf,
		.count = ARRAY_SIZE(line_buf),
	};
	const uint32_t end_line = (uint32_t)start_line + num_lines;
	int err;
	int release_err;

	LOG_DBG("Lines %u to %u", (unsigned int)start_line, (unsigned int)(end_line - 1U));

	err = k_mutex_lock(&ls0xx_bus_mutex, K_MSEC(LS0XX_MAX_BUS_WAIT_MSEC));
	if (err < 0) {
		LOG_ERR("LS0xx bus mutex unavailable for refresh: %d", err);
		return err;
	}

	err = ls0xx_cmd(dev, write_cmd, sizeof(write_cmd));
	if (err == 0) {
		for (uint32_t line = start_line; line < end_line; line++) {
			line_number = (uint8_t)line;
			line_buf[1].buf = (uint8_t *)data;
			err = spi_write_dt(&config->bus, &line_set);
			if (err < 0) {
				LOG_ERR("LS0xx line %u write failed: %d", (unsigned int)line, err);
				break;
			}
			data += LS0XX_PANEL_WIDTH / LS0XX_PIXELS_PER_BYTE;
		}
	}

	if (err == 0) {
		/* The panel requires one trailing byte while CS remains asserted. This
		 * is data, not a new command, so it must not advance VCOM polarity. */
		err = spi_write_dt(&config->bus, &trailing_set);
		if (err < 0) {
			LOG_ERR("LS0xx trailing byte write failed: %d", err);
		}
	}

	release_err = ls0xx_release_bus(dev);
	return err < 0 ? err : release_err;
}

static int ls0xx_write(const struct device *dev, const uint16_t x, const uint16_t y,
			const struct display_buffer_descriptor *desc, const void *buf)
{
	if (desc == NULL) {
		LOG_ERR("Display buffer descriptor is not available");
		return -EINVAL;
	}

	LOG_DBG("X %u, Y %u, W %u, H %u", (unsigned int)x, (unsigned int)y,
		(unsigned int)desc->width, (unsigned int)desc->height);

	if (buf == NULL) {
		LOG_WRN("Display buffer is not available");
		return -EINVAL;
	}

	if (desc->width != LS0XX_PANEL_WIDTH) {
		LOG_ERR("Width must equal %u", (unsigned int)LS0XX_PANEL_WIDTH);
		return -EINVAL;
	}

	if (desc->pitch != desc->width) {
		LOG_ERR("Unsupported pitch");
		return -ENOTSUP;
	}

	if (((uint32_t)y + desc->height) > LS0XX_PANEL_HEIGHT) {
		LOG_ERR("Buffer is out of bounds");
		return -EINVAL;
	}

	if (x != 0U) {
		LOG_ERR("X coordinate must be zero");
		return -EINVAL;
	}

	if (desc->height == 0U) {
		return 0;
	}

	/* Panel line numbering starts at one. */
	return ls0xx_update_display(dev, y + 1U, desc->height, buf);
}

static void ls0xx_get_capabilities(const struct device *dev, struct display_capabilities *caps)
{
	ARG_UNUSED(dev);

	memset(caps, 0, sizeof(*caps));
	caps->x_resolution = LS0XX_PANEL_WIDTH;
	caps->y_resolution = LS0XX_PANEL_HEIGHT;
	caps->supported_pixel_formats = PIXEL_FORMAT_MONO01;
	caps->current_pixel_format = PIXEL_FORMAT_MONO01;
	caps->screen_info = SCREEN_INFO_X_ALIGNMENT_WIDTH;
}

static int ls0xx_set_pixel_format(const struct device *dev, const enum display_pixel_format pf)
{
	ARG_UNUSED(dev);

	if (pf == PIXEL_FORMAT_MONO01) {
		return 0;
	}

	LOG_ERR("Unsupported pixel format");
	return -ENOTSUP;
}

static int ls0xx_init(const struct device *dev)
{
	const struct ls0xx_config *config = dev->config;
	struct ls0xx_data *data = dev->data;
	int err;

	if (!spi_is_ready_dt(&config->bus)) {
		LOG_ERR("SPI bus %s is not ready", config->bus.bus->name);
		return -ENODEV;
	}

#if DT_INST_NODE_HAS_PROP(0, disp_en_gpios)
	if (!gpio_is_ready_dt(&config->disp_en_gpio)) {
		LOG_ERR("Display-enable GPIO is not ready");
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&config->disp_en_gpio, GPIO_OUTPUT_HIGH);
	if (err < 0) {
		LOG_ERR("Failed to configure display-enable GPIO: %d", err);
		return err;
	}
#endif

#if DT_INST_NODE_HAS_PROP(0, extcomin_gpios)
	if (!gpio_is_ready_dt(&config->extcomin_gpio)) {
		LOG_ERR("EXTCOMIN GPIO is not ready");
		return -ENODEV;
	}

	err = gpio_pin_configure_dt(&config->extcomin_gpio, GPIO_OUTPUT_LOW);
	if (err < 0) {
		LOG_ERR("Failed to configure EXTCOMIN GPIO: %d", err);
		return err;
	}
#endif

	data->vcom_state = false;

	/* Clear before starting the periodic thread so initialization owns the
	 * first complete SPI transaction.
	 */
	err = ls0xx_clear(dev);
	if (err < 0) {
		return err;
	}

#ifdef USE_VCOM_THREAD
	k_tid_t vcom_toggle_tid = k_thread_create(
		&vcom_toggle_thread, vcom_toggle_stack, K_THREAD_STACK_SIZEOF(vcom_toggle_stack),
		ls0xx_vcom_toggle, (void *)dev, NULL, NULL, LS0XX_VCOM_PRIO, 0, K_NO_WAIT);

	err = k_thread_name_set(vcom_toggle_tid, "ls0xx_vcom");
	if (err < 0) {
		LOG_WRN("Failed to name VCOM thread: %d", err);
	}
#endif

	return 0;
}

static struct ls0xx_data ls0xx_dev_data;

static const struct ls0xx_config ls0xx_config = {
	.bus = SPI_DT_SPEC_INST_GET(0,
				    SPI_OP_MODE_MASTER | SPI_WORD_SET(8) | SPI_TRANSFER_LSB |
					    SPI_CS_ACTIVE_HIGH | SPI_HOLD_ON_CS | SPI_LOCK_ON,
				    0),
#if DT_INST_NODE_HAS_PROP(0, disp_en_gpios)
	.disp_en_gpio = GPIO_DT_SPEC_INST_GET(0, disp_en_gpios),
#endif
#if DT_INST_NODE_HAS_PROP(0, extcomin_gpios)
	.extcomin_gpio = GPIO_DT_SPEC_INST_GET(0, extcomin_gpios),
#endif
#if DT_INST_PROP(0, serial_vcom_inversion)
	.serial_vcom_int = DT_INST_PROP(0, serial_vcom_interval),
#endif
};

static DEVICE_API(display, ls0xx_driver_api) = {
	.blanking_on = ls0xx_blanking_on,
	.blanking_off = ls0xx_blanking_off,
	.write = ls0xx_write,
	.get_capabilities = ls0xx_get_capabilities,
	.set_pixel_format = ls0xx_set_pixel_format,
};

DEVICE_DT_INST_DEFINE(0, ls0xx_init, NULL, &ls0xx_dev_data, &ls0xx_config, POST_KERNEL,
			      CONFIG_DISPLAY_INIT_PRIORITY, &ls0xx_driver_api);
