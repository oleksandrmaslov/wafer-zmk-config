# SPDX-License-Identifier: MIT

zephyr_library_named(zmk_npm1300_vbat)
zephyr_library_sources_ifdef(CONFIG_ZMK_NPM1300_VBAT npm1300_vbat.c)
