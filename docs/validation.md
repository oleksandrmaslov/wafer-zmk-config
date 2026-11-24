# Firmware cleanup validation

Validated locally on 2026-11-10 and revalidated on 2026-11-11 after fixing
Linux BLE Raw HID report framing. No firmware was flashed by these checks.

| Build | Result |
| --- | --- |
| `wafer_left`, default manifest | UF2 linked successfully |
| `wafer_right`, default manifest | UF2 linked successfully |
| `wafer_left`, HID manifest + `wafer-hid-display` snippet | UF2 linked successfully |
| `wafer_right`, HID manifest + `wafer-hid-display` snippet | UF2 linked successfully |

All four builds passed `tests/check_builds.py`, checking the actual generated
Kconfig and devicetree artifacts for:

- enabled split soft off, button gesture, hold-to-wake and inactivity wake guard;
- inverted shutdown blanking explicitly enabled by the Wafer board;
- correct active-low P0.03 power/wake GPIO and 1500 ms timings;
- mirrored right-half columns and its five-column transform offset;
- consistency between the keymap editor matrix positions and ZMK transform;
- instantiated battery sensor and lithium-voltage reporting;
- serial VCOM inversion and the 33 ms display interval;
- three standard QWERTY/NUMBER/SYMBOL layers with 36 bindings each;
- mutually exclusive local/HID display widgets and Raw HID/relay isolation.

The original stock keymap bindings were retained. The duplicate user keymap was
removed so builds fall back to the board's canonical keymap.

GitHub workflow validation passed `actionlint` 1.7.7. YAML parsing, manifest
consistency, board matrices, and `git diff --check` also passed.

## Effective inputs

- ZMK: `4dd95d7840eb3d5a3f59bb29fbf1ab054715ef85`
- Zephyr: `10ba6d0cb38bc3d258775d27982f707599320085` (resolved from ZMK's imported
  `v4.1.0+zmk-fixes` reference; that imported reference is not an immutable pin)
- soft-off-plus: `42784a454ca21ccd46c99497cca6af7d0df4bf13`
- optional HID module: `b5c00042bce37f02b1b70b9e7d3b6913f013e7a6`
- Python 3.12, CMake 3.30.5, west 1.5.0, ARM GCC 14.3.0

The installed Zephyr SDK 1.0.1 is newer than Zephyr 4.1's supported SDK package
range. Local validation used its ARM compiler through the supported
`ZEPHYR_TOOLCHAIN_VARIANT=cross-compile` / `CROSS_COMPILE` settings rather than
changing project code or the SDK. GitHub Actions continues to use ZMK's build
container. The default left/right GitHub builds also passed after publication at
`2460a77`: [push build](https://github.com/oleksandrmaslov/wafer-zmk-config/actions/runs/42880827879)
and [manual build](https://github.com/oleksandrmaslov/wafer-zmk-config/actions/runs/42883512967).
The optional HID job was skipped in those runs; its two local builds passed.

Existing KSCAN/BT_CTLR deprecation warnings remain. The local central display
widget also has an existing switch warning for `ZMK_TRANSPORT_NONE`.

On 2026-11-24, the module-wide inverted blanking default and the later early
Nordic wake guard were reverted after the startup display blinking persisted on
hardware. The soft-off module tree again matches `822aa0b`; Wafer explicitly
enables inverted blanking in the board configuration. Personal firmware keeps
its original explicit `CONFIG_ZMK_SOFT_OFF_PLUS_BLANK_INVERTED=y` setting.
The separate Raw HID framing fix remains pinned and unchanged.

The HID profile and personal left/right builds also passed after the BLE framing
fix. The module's regression check replays native reports and Linux-style
zero-prefixed reports through the real BLE write callback, checking payload
alignment, write return lengths, invalid offsets and report sizes 1, 32 and 255.
The HID descriptor and USB receive path are unchanged.

On the laptop, the original companion's 33-byte write reproduced the Bluetooth
attribute-length error, while the same 32-byte payload passed. The local companion
update was tested against the running KDE Wayland session: it mapped all four
layouts to the firmware's label order and sent time, volume and layout at startup
in three full-app trials. The desktop's original layout was restored. This tests
the companion and receive callback separately; delivery to the physical displays
still requires flashing the new central firmware. macOS and Windows were not
tested on hardware.

Locally built UF2 files are in ignored `build/validated/standard/` and
`build/validated/hid-display/`. Runtime power behavior, display rendering,
Bluetooth stability, and real battery readings still require a keyboard check.

## Display power during rejected wake attempts (2026-11-24)

The right-half application was read back through `CURRENT.UF2` and matched the
personal rollback image byte for byte. Verified erasure of all eight settings
pages and a separate nPM1300 full power reset did not remove the reported noise.
The PMIC reset cleared its retained load-switch state, then normal driver
initialization restored all 11 sampled registers to their previous values.

GPIO0 retained its factory pull-down (`GPIOPDEN[0] = 1`, `GPIOPUEN[0] = 0`).
Both ends of the display enable wire were configured active-low, so a released
MCU pin could request display power before the wake hold was confirmed. The
[bootloader's base revision](https://github.com/adafruit/Adafruit_nRF52_Bootloader/blob/91df8bd/src/boards/boards.c#L156)
resets all GPIOs before handing control to the application. Nordic documents
the pull-down default in the [nPM1300 GPIO specification](https://docs.nordicsemi.com/r/bundle/ps_npm1300/page/chapters/core_components/gpio/doc/frontpage.html).

P0.29 and the PMIC GPIO0 enable input now both use active-high polarity. The
default low level keeps the load switch off through bootloader handoff. The
soft-off module, inverted shutdown blank, hold timing and Raw HID framing fix
are unchanged. A test overlay changed only these two devicetree properties;
its Kconfig was identical to the previous personal build.

The user tested the right half on battery: three brief taps remained free of
noise after allowing for MIP retention, and a full hold showed the normal image.
`tests/check_power_reset_bias.py` rejects the previous configuration and checks
that the real generated devicetrees keep display power off when the MCU releases
the enable line. It also checks that both ends agree on polarity and is run by
`tests/check_builds.py`. Left-half runtime behavior and current draw still need
hardware checks.
