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
