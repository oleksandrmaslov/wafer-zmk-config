"""Check locally generated Wafer builds: python check_builds.py ZEPHYR_BASE BUILD..."""

import json
import pickle
from pathlib import Path
import sys

zephyr, *builds = map(Path, sys.argv[1:])
assert builds, "Pass at least one firmware build directory"
sys.path.insert(0, str(zephyr / "scripts/dts/python-devicetree/src"))

root = Path(__file__).resolve().parents[1]
editor_keys = json.loads((root / "config/wafer.json").read_text())["layouts"]["Wafer Layout"]["layout"]

for build in builds:
    config = dict(line.split("=", 1) for line in (build / "zephyr/.config").read_text().splitlines()
                  if line.startswith("CONFIG_"))
    # Read only locally generated Zephyr build artifacts.
    with (build / "zephyr/edt.pickle").open("rb") as source:
        tree = pickle.load(source)
    nodes = tree.label2node
    central = config.get("CONFIG_ZMK_SPLIT_ROLE_CENTRAL") == "y"
    hid = config.get("CONFIG_NICE_VIEW_HID") == "y"
    assert (build / "zephyr/zmk.uf2").is_file(), f"{build}: firmware did not finish building"
    for name in ("ZMK_PM_SOFT_OFF", "ZMK_NPM1300_VBAT", "ZMK_BATTERY_REPORTING_FETCH_MODE_LITHIUM_VOLTAGE",
                 "ZMK_SOFT_OFF_PLUS_SPLIT_SYNC", "ZMK_SOFT_OFF_PLUS_POWER_BUTTON",
                 "ZMK_SOFT_OFF_PLUS_WAKE_DELAY", "ZMK_SOFT_OFF_PLUS_DEEP_SLEEP_WAKE",
                 "ZMK_SOFT_OFF_PLUS_BLANK_INVERTED"):
        assert config.get("CONFIG_" + name) == "y", (build, name)
    assert config.get("CONFIG_BT_CTLR_PHY_2M") != "y"
    assert (config.get("CONFIG_NICE_VIEW_WIDGET_STATUS") == "y") != hid
    assert (config.get("CONFIG_RAW_HID") == "y") == hid
    assert ("rawhid_or" in nodes) == hid
    if hid:
        assert config["CONFIG_USB_HID_DEVICE_COUNT"] == "2"
        assert nodes["rawhid_or"].props["device"].val == nodes["display"]

    def gpio(label, prop):
        return [(g.controller.labels[0], g.data["pin"], g.data["flags"])
                for g in nodes[label].props[prop].val]

    assert gpio("soft_off_gpio_key", "gpios") == [("gpio0", 3, 17)]
    assert gpio("soft_off_plus_wake", "wake-gpios") == gpio("soft_off_gpio_key", "gpios")
    assert nodes["soft_off_plus_wake"].props["wake-hold-ms"].val == 1500
    assert nodes["hw_soft_off"].props["hold-time-ms"].val == 1500
    columns = [("gpio1", 14, 0), ("gpio1", 15, 0), ("gpio1", 6, 0), ("gpio1", 2, 0), ("gpio0", 8, 0)]
    assert gpio("kscan0", "col-gpios") == (columns if central else columns[::-1])
    transform = nodes["default_transform"].props
    assert transform["col-offset"].val == (0 if central else 5)
    assert transform["map"].val == [k["row"] * 256 + k["col"] for k in editor_keys]
    assert tree.chosen_node("zmk,battery") == nodes["npm1300_vbat_sensor"]
    assert nodes["display"].props["serial-vcom-inversion"].val
    assert nodes["display"].props["serial-vcom-interval"].val == 33
    layers = list(tree.get_node("/keymap").children.values())
    assert [layer.props["display-name"].val for layer in layers] == ["QWERTY", "NUMBER", "SYMBOL"]
    assert all(len(layer.props["bindings"].val) == 36 for layer in layers)
    print(f"{build.name}: power, matrix, battery, standard keymap and HID isolation passed")
