# Firmware cleanup validation

Validated locally on 2026-11-10 and revalidated on 2026-11-11 after moving
inverted shutdown blanking into the module's defaults and fixing Linux BLE
Raw HID report framing. No firmware was flashed to hardware.

| Build | Result |
| --- | --- |
| `wafer_left`, default manifest | UF2 linked successfully |
| `wafer_right`, default manifest | UF2 linked successfully |
| `wafer_left`, HID manifest + `wafer-hid-display` snippet | UF2 linked successfully |
| `wafer_right`, HID manifest + `wafer-hid-display` snippet | UF2 linked successfully |

All four builds passed `tests/check_builds.py`, checking the actual generated
Kconfig and devicetree artifacts for:

- enabled split soft off, button gesture, hold-to-wake and inactivity wake guard;
- inverted shutdown blanking enabled by the module, without a Wafer override;
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
- soft-off-plus: `e24d18023b72bca9a53e737decd1123851823c8e`
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

Clean configuration checks also verified the module's inverted default for
`main` and `personal-layout` with both Wafer overrides removed, and confirmed
that an explicit `n` remains supported for non-inverted display pipelines.
The power/wake device trees and effective power settings still match the
personal reference. The reported battery-powered short/double-tap wake has not
been reproduced on hardware; this change only moves the blanking default.

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
