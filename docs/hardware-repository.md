# Wafer keyboard repository and licensing

The PCB and case sources are in the separate
[wafer-keyboard repository](https://github.com/oleksandrmaslov/wafer-keyboard).
All 54 PCB/case files were verified byte-for-byte against the original firmware
main before removal from this firmware working tree. Hardware-related history
and historical licence records are retained; no PCB or case geometry was changed.

## Current licences

| Material | Licence |
| --- | --- |
| Firmware code, keymaps, devicetrees including physical-layout data, configuration, and software documentation | MIT in the firmware root `LICENSE`. |
| Photos, artwork, renders, and the illustrated layout | CC BY-NC 4.0 in `assets/LICENSE`; attribution in `assets/README.md`. |
| PCB/CAD design files and hardware documentation authored by Oleksandr Maslov | CC BY-NC 4.0 in `wafer-keyboard/LICENSE`. |

The full, unmodified CC BY-NC 4.0 text is included. The standard MIT text retains
the ZMK Contributors' and Oleksandr Maslov's copyright notices. Redundant
`LICENSE-CODE.md`, `LICENSE-HARDWARE.md`, and `LICENSE-NOTICE.md` files were removed
from the current trees. Historical grants and third-party notices remain valid;
this cleanup does not revoke earlier permissions.

## Personal builds and commercial production

The hardware repository's `MANUFACTURING.md` explicitly permits personal builds,
modifications, and paying a PCB fabricator or CNC shop to make your personal
copies. The contractor may charge for fulfilling that personal order. Commercial
production, kits, commercial group buys, and paid builds for sale require
separate written permission from Oleksandr Maslov. The notice grants additional
personal-fabrication permission and leaves the standard CC licence unchanged.

[CC BY-NC 4.0](https://creativecommons.org/licenses/by-nc/4.0/) permits credited,
noncommercial sharing and adaptations. Attribution includes a licence link and
identifying changes. Its standard definition of NonCommercial applies; no new
unit limits or custom definition were added.

CC applies here to design files and documentation. Creative Commons
[does not recommend its licences as a complete hardware-rights solution](https://creativecommons.org/faq/),
so the explicit fabrication permission addresses the owner's copyright/design
rights where required. No patent or trademark licence is granted. Third-party
footprints, models, and contributions keep their own applicable terms. A
noncommercial restriction is incompatible with the
[OSHWA open hardware definition](https://oshwa.org/resources/open-source-hardware-definition/);
this is a source-available design with personal-use permission.

## Repository split and history

`wafer-keyboard/pcb/` contains the original KiCad sources, BOM, fabrication files,
and backups. `wafer-keyboard/case/` contains the CNC STEP models and thread
drawing. Photographs and firmware layout data remain in the firmware repository.

The firmware history cleanup removes only `pcb/` and `case/` from historical
trees. It preserves firmware changes and keeps a full old-to-new commit map and
a verified pre-cleanup bundle. Obsolete hardware-licence files remain in older
commits as historical licence records. Future hardware-only files belong in the
keyboard-design repository.

The hardware repository is published separately from the firmware. The external
ZMK modules remain in their own repositories.
