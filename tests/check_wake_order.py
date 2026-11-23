"""Check the real firmware initializes its wake guard before display-related I/O."""

from pathlib import Path
import re
import subprocess
import sys

zephyr = Path(sys.argv[1]).resolve()
for build in map(Path, sys.argv[2:]):
    order = subprocess.check_output(
        [sys.executable, str(zephyr / "scripts/build/check_init_priorities.py"),
         "-f", str(build / "zephyr/zmk.elf"), "-i"], text=True,
    )
    functions = re.findall(r": (\w+)\(", order)
    guard = functions.index("wake_delay_init")
    assert functions.index("gpio_nrfx_init") < guard, "wake GPIO is not ready"
    for name in ("spi_nrfx_init", "usb_init", "ext_power_generic_init", "ls0xx_init"):
        if name in functions:
            assert guard < functions.index(name), f"{build.name}: {name} runs before the wake hold guard"
    print(f"{build.name}: wake hold is checked before SPI, USB and display power/init")
