"""Check Wafer's load switch stays off when the bootloader releases MCU GPIOs."""

from pathlib import Path
import pickle
import sys

zephyr, *builds = map(Path, sys.argv[1:])
assert builds, "Pass the Zephyr source and at least one firmware build directory"
sys.path.insert(0, str(zephyr.resolve() / "scripts/dts/python-devicetree/src"))

for build in builds:
    # Read only locally generated Zephyr build artifacts.
    with (build / "zephyr/edt.pickle").open("rb") as source:
        tree = pickle.load(source)
    native = tree.compat2okay["zmk,ext-power-generic"][0].props["control-gpios"].val[0]
    pmic = tree.label2node["npm1300_ldo1"].props["enable-gpios"].val[0]
    assert native.controller == tree.label2node["gpio0"] and native.data["pin"] == 29
    assert pmic.controller == tree.label2node["npm1300_gpio"] and pmic.data["pin"] == 0
    native_inverted = bool(native.data["flags"] & 1)
    pmic_inverted = bool(pmic.data["flags"] & 1)
    assert native_inverted == pmic_inverted, "MCU and PMIC disagree on the enable polarity"
    # GPIOPDEN[0] resets to 1; verified on Wafer before and after a PMIC reset.
    released_level = False
    power_requested = released_level != pmic_inverted
    assert not power_requested, f"{build.name}: releasing MCU GPIO requests display power"
    print(f"{build.name}: released control line keeps the display load switch off")
