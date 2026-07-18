# SPDX-License-Identifier: MIT
# Top-level CMake stub so this repo can act as a Zephyr module that
# contributes the boards/, dts/, display driver, and npm1300_vbat sensor.

if(CONFIG_ZMK_NPM1300_VBAT)
  add_subdirectory(modules/npm1300_vbat)
endif()

if(CONFIG_WAFER_LS0XX_SAFE)
  add_subdirectory(modules/wafer_ls0xx)
endif()
