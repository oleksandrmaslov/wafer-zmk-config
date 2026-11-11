# Building and flashing Wafer firmware

Wafer uses standalone `wafer_left` (central) and `wafer_right` (peripheral) boards.
The default build has the standard QWERTY keymap and the required soft-off module.
The optional HID display profile adds host information and its split relay.

## Default builds

Push to your fork or run the **build** GitHub Actions workflow for the default
`wafer_left` and `wafer_right` UF2 files. The board's `wafer.keymap` is the single
stock keymap. To customize it, copy it to `config/wafer.keymap`; user keymaps
override the board default. Put ordinary Kconfig overrides in `config/wafer.conf`.

For a local build from a fresh directory with the ZMK toolchain installed:

```sh
mkdir wafer-workspace
cd wafer-workspace
git clone https://github.com/oleksandrmaslov/wafer-zmk-config.git wafer
mkdir config
cp wafer/config/west.yml config/west.yml
west init -l config
west update
west build -s zmk/app -d build/left -b wafer_left -- -DZMK_CONFIG="$PWD/wafer/config" -DZMK_EXTRA_MODULES="$PWD/wafer"
west build -s zmk/app -d build/right -b wafer_right -- -DZMK_CONFIG="$PWD/wafer/config" -DZMK_EXTRA_MODULES="$PWD/wafer"
```

## Optional HID display

This profile enables host-driven nice!view data and the split Raw HID relay.
On GitHub, run **build** manually with **hid_display** checked. Pull requests
also compile this profile to check it alongside the default firmware; its UF2s
are in the separate `firmware-hid-display` archive.

For a separate local workspace, initialize with the optional manifest:

```sh
mkdir wafer-hid-workspace
cd wafer-hid-workspace
git clone https://github.com/oleksandrmaslov/wafer-zmk-config.git wafer
mkdir config
cp wafer/config/hid-display/west.yml config/west.yml
west init -l config
west update
west build -s zmk/app -d build/hid-left -b wafer_left -S wafer-hid-display -- -DZMK_CONFIG="$PWD/wafer/config/hid-display" -DZMK_EXTRA_MODULES="$PWD/wafer"
west build -s zmk/app -d build/hid-right -b wafer_right -S wafer-hid-display -- -DZMK_CONFIG="$PWD/wafer/config/hid-display" -DZMK_EXTRA_MODULES="$PWD/wafer"
```

The two manifests pin the same ZMK and soft-off revisions. Keep these pins in
sync when upgrading. The optional profile additionally pins the HID module.
Set `CONFIG_NICE_VIEW_HID_LAYOUTS` in the snippet to match your host app's language
order (default `US`). Custom keymap/config overrides for this profile belong in
`config/hid-display/`. Normal builds have no Raw HID interface or relay.

For USB logs, add `-S zmk-usb-logging` to a default local build. To recover bonds,
build each half with `-DCONFIG_ZMK_SETTINGS_RESET_ON_START=y`, boot both recovery
images once, then flash normal firmware. Recovery images erase persistent
settings on every boot; keep them separate from ordinary firmware.

After building, verify the generated firmware configuration and device tree with
`python tests/check_builds.py /path/to/zephyr /path/to/build/left /path/to/build/right`
from the firmware repository, using your build environment's Python. Include the
HID build directories in the same check when built.

## Flashing

Connect the half you want to flash by USB. Triple-tap its dedicated power button
to enter the UF2 bootloader, then copy the matching left or right UF2 to the
bootloader drive. Repeat for the other half with its own firmware file.

After a deliberate shutdown on battery, hold each half's power button for
1.5 seconds to wake it. The board's `bypass-on-usb` setting permits a short
button press instead when USB power is detected. Wake the half before using
the triple-tap bootloader gesture. A half in System OFF cannot wake the other
half over Bluetooth.

See the [board file guide](../boards/waferboard/wafer/README.md),
[validation record](validation.md), and [root README](../README.md).
