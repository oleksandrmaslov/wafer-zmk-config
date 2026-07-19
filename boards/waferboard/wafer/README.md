# Wafer ZMK Board

This directory contains the Zephyr/ZMK board definition for the **Wafer** split keyboard. The layout follows the standard ZMK board structure (Zephyr Hardware Model v2), so files cannot be rearranged without breaking the build. Below is a classification of each file by what it does.

## Build system & board metadata

| File | Purpose |
| --- | --- |
| `CMakeLists.txt` | Zephyr CMake entry point - pulls board sources into the build. |
| `board.cmake` | Board-level CMake hooks (flashing/runner config). |
| `pre_dt_board.cmake` | Hooks run before devicetree generation. |
| `board.yml` | Hardware Model v2 board metadata (qualifiers, SoC, variants). |
| `wafer.yaml` | Legacy Zephyr board YAML (board name, supported features). |
| `wafer.zmk.yml` | ZMK-specific board metadata (siblings, features, exposed in ZMK Studio / GitHub Actions). |

## Kconfig (build-time options)

| File | Purpose |
| --- | --- |
| `Kconfig` | Top-level board Kconfig - declares board symbols. |
| `Kconfig.defconfig` | Default Kconfig values applied when this board is selected. |
| `Kconfig.wafer_left` | Kconfig overrides specific to the left half. |
| `Kconfig.wafer_right` | Kconfig overrides specific to the right half. |
| `wafer_defconfig` | Common Kconfig defaults compiled into both halves. |
| `wafer_left_defconfig` | Kconfig defaults for the left-half build. |
| `wafer_right_defconfig` | Kconfig defaults for the right-half build. |

## Devicetree (hardware description)

| File | Purpose |
| --- | --- |
| `wafer.dtsi` | Shared devicetree include - common hardware (MCU, peripherals, matrix). |
| `wafer-pinctrl.dtsi` | Pin control / pinmux definitions for the SoC peripherals. |
| `wafer_left.dts` | Devicetree for the left half (column offset, side-specific nodes). |
| `wafer_right.dts` | Devicetree for the right half (column offset, side-specific nodes). |

## ZMK firmware configuration

| File | Purpose |
| --- | --- |
| `wafer.conf` | Shared ZMK/Kconfig flags applied to both halves at build time. |
| `wafer.keymap` | Keymap (layers, behaviors, combos) - the user-facing layout. |

## Display / UI

| File | Purpose |
| --- | --- |
| `custom_status_screen.c` | Custom nice!view status screen rendering (LVGL widgets). |
| `widgets/` | Supporting widget sources used by the custom status screen. |

## Notes

Most of these files follow conventions documented in the [ZMK docs](https://zmk.dev/docs) and the [Zephyr Hardware Model v2](https://docs.zephyrproject.org/latest/hardware/porting/board_porting.html). They are required to live in this exact folder structure for the ZMK build system to discover the board.


## Battery / nPM1300 VBAT

This board reports battery state from a Nordic **nPM1300** PMIC. The standard ZMK `nordic,npm1300-charger` battery binding only exposes charger state, not VBAT, so we ship a small custom Zephyr sensor driver that triggers a VBAT measurement on the PMIC ADC and reports the voltage on `SENSOR_CHAN_VOLTAGE` in millivolts (`SENSOR_CHAN_GAUGE_VOLTAGE` is also accepted for compatibility). ZMK then converts that to a battery percentage through its lithium-voltage curve.

The driver lives as a ZMK module in this repo at `modules/npm1300_vbat/` (devicetree binding `zmk,npm1300-vbat`, Kconfig `CONFIG_ZMK_NPM1300_VBAT`). The board's `wafer.dtsi` declares an `npm1300_vbat_sensor` node that takes a phandle to the existing `pmic` (`pmic@6b`) node, and `zmk,battery` in `chosen` points at this sensor. Required Kconfig flags live in `wafer.conf`: `CONFIG_ZMK_BATTERY_REPORTING=y`, `CONFIG_ZMK_BATTERY_REPORTING_FETCH_MODE_LITHIUM_VOLTAGE=y`, `CONFIG_ZMK_NPM1300_VBAT=y`.



## Building

The board pins ZMK to `c77aa1c877cfd1e55c7733e2207affa1a90bc9aa` in
`config/west.yml`. ZMK's imported manifest supplies its matching Zephyr and
module dependencies, so this configuration does not declare Zephyr a second
time. Dependency SHAs and the reusable workflow pin are updated deliberately
only after the complete Wafer build matrix passes. GitHub Actions in
`.github/workflows/build.yml` runs on pushes, pull requests, and manual
dispatches and produces the left/right firmware artifacts.
