# Wafer ZMK board

Both halves are standalone nRF52840/ISP1807 boards. Use `wafer_left` (central)
and `wafer_right` (peripheral); these existing board names and paths are retained.

| File | Purpose |
| --- | --- |
| `wafer_left.dts`, `wafer_right.dts` | Per-half column GPIO order; right-half transform offset. |
| `wafer.dtsi` | Common MCU, matrix rows, transform, chosen devices, LED, external power, flash partitions. |
| `wafer-pmic.dtsi` | nPM1300 I2C, regulators, charger, PMIC LEDs, VBAT sensor. |
| `wafer-display.dtsi` | nice!view SPI and serial VCOM timing. |
| `wafer-power.dtsi` | Dedicated sideband power button, split soft off, USB-only bootloader gesture, hold-to-wake. |
| `wafer-pinctrl.dtsi` | Peripheral pin multiplexing. |
| `wafer_left_defconfig`, `wafer_right_defconfig` | Hardware capabilities for each board. |
| `Kconfig.defconfig` | Shared defaults, display widget selection and tested power/radio settings. |
| `wafer.conf` | Battery reporting defaults for configurations without user overrides. |
| `wafer.keymap` | Canonical standard 36-key QWERTY layout. |
| `widgets/`, `custom_status_screen.c` | Local status display used by the default build. |

The root `zephyr/module.yml` registers this board, `dts/` and `snippets/`.
Root Kconfig/CMake register the local `modules/npm1300_vbat` driver exactly once.
Its single binding lives at `dts/bindings/sensor/zmk,npm1300-vbat.yaml`; the board
chooses `npm1300_vbat_sensor` as `zmk,battery`. The driver reports volts through
`SENSOR_CHAN_VOLTAGE` (and the compatible gauge channel); ZMK converts voltage to
battery percentage using its lithium curve. The driver currently expects the
PMIC label `pmic`, as provided by this board.

The external `zmk-soft-off-plus` module is required by the board's dedicated
power-button tree. `zmk-split-hid-display` is optional: its relay node and feature
configuration live entirely in `snippets/wafer-hid-display/`.
See the [root README](../../../README.md) for build instructions.
