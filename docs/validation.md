# Firmware cleanup validation

Validated locally on 2026-11-10. No firmware was flashed to hardware.

| Build | Result |
| --- | --- |
| `wafer_left`, default manifest | UF2 linked successfully |
| `wafer_right`, default manifest | UF2 linked successfully |
| `wafer_left`, HID manifest + `wafer-hid-display` snippet | UF2 linked successfully |
| `wafer_right`, HID manifest + `wafer-hid-display` snippet | UF2 linked successfully |

All four builds passed `tests/check_builds.py`, checking the actual generated
Kconfig and devicetree artifacts for:

- enabled split soft off, button gesture, hold-to-wake and inactivity wake guard;
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
- soft-off-plus: `822aa0b6c970ec08b19ab0ba056098dcba6bfa46`
- optional HID module: `ce04bf6e90f71ecae69e6c6b9f75522cb5578327`
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

Locally built UF2 files are in ignored `build/validated/standard/` and
`build/validated/hid-display/`. Runtime power behavior, display rendering,
Bluetooth stability, and real battery readings still require a keyboard check.
