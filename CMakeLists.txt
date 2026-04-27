# SPDX-License-Identifier: MIT
# Top-level CMake stub so this repo can act as a Zephyr module that
# contributes the boards/, dts/, and the npm1300_vbat sensor module.

if(CONFIG_ZMK_NPM1300_VBAT)
  add_subdirectory(modules/npm1300_vbat)
endif()
