
# Wafer

![Wafer title photo](/assets/wafer_title.png)

Wafer is a 36 key ultra-thin split keyboard that stays between 4–8 mm thick. It runs on an ISP1807 with an NPM1300 PMIC, and the full aluminium case snaps together with magnets for a clean profile. Runs ZMK for ultra-low-power

***

## LAYOUT

![WAFER layout](/assets/wafer_layout_default.svg)

Default layout is shown above; layers and firmware live in this repo.

***

## PCB

[*Here*](/pcb/) you could find the PCB and production files for the Wafer. There are two versions, with mousebites and ready for production and without them.

***

## BOM

The full Bill of Materials for the Wafer keyboard is available in the repo:

- [BOM (JLCPCB production)](pcb/jlcpcb/production_files/BOM-wafer_keyboard_v3_production.csv)
- [Keyboard Price breakdown](pcb/Keyboard%20Price.csv)
***

## CASE

These models are mostly for CNC right now; a tuned 3D-printable and CNC version will be shared soon.
***


***

## PHOTOS
![side view](/assets/IMG_3287.JPG)
![](/assets/IMG_3291.JPG)
![Wafer PCB](/assets/IMG_3320.JPG)
![Art on the back](/assets/IMG_3324.JPG)
![Art in KiCAD](/assets/Back_art.png)
![Wafer render](/assets/wafer%20render.jpg)


***

## LICENSE

This repository uses a split-license model.

### Software source code and firmware configuration

Software source code and firmware configuration are licensed under the MIT License. See [LICENSE-CODE.md](LICENSE-CODE.md).

### Hardware, PCB, CAD, wafer, case, visuals, and product design

All hardware designs, PCB files, CAD files, Gerber files, 3D models, enclosure designs, wafer designs, layout geometry, visual assets, renders, documentation, product identity, brand elements, and the Wafer keyboard concept are copyright (c) 2026 Oleksandr Maslov. All rights reserved unless explicitly stated otherwise.

These non-software materials are shared only for viewing, documentation, discussion, and personal non-commercial reference. See [LICENSE-HARDWARE.md](LICENSE-HARDWARE.md).

You may share links to this repository.

You may not copy, manufacture, sell, redistribute, remix, modify, reupload, clone for production, or commercially use the hardware/design/wafer files without written permission.

### License clarification

The MIT License applies only to software source code and firmware configuration. It does not apply to hardware designs, wafer designs, keyboard layout geometry, PCB files, CAD files, Gerber files, 3D models, visual assets, documentation, product identity, or the Wafer keyboard concept.

***

## CREDITS

### INSPIRATION

Inspired by the [Totem Keyboard](https://github.com/GEIGEIGEIST/TOTEM) and [Mikehive](https://github.com/mikeholscher/zmk-config-mikefive), but with a slimmer profile and magnets for usability. Big thanks to Rasmus for introducing me to the ISP1807.

### HELP FIXING THINGS

People who helped me create this board and fix stuff:

#### PCB
- [Rasmus Koit](https://github.com/rasmuskoit)
- [Eden](https://github.com/galaxyeden)
#### CASE
- [Vlad Vodkin (consulted me to build this case, huge thanks)](https://t.me/vlad30303)

