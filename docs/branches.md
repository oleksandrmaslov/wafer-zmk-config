# Branch review

Reviewed against `origin/personal-layout` at `fb8d5267b551f9bd403eb09a3b02cf4285906a60`
on 2026-11-10, after fetching origin. No open pull requests were found.

| Branch at review | Commits outside personal-layout | Assessment |
| --- | ---: | --- |
| `main` | 0 (34 commits behind) | Keep as the standard public firmware configuration. |
| `personal-layout` | baseline | Keep: the daily-use layout and tested reference. Latest build passed 2026-11-04. |
| `debug2` | 0 (18 commits behind) | Safe to remove: the entire branch is an ancestor of personal-layout. |
| `codex/fix-last-actions-build-error` | 3 | Superseded debugging approach; archive before removing. It removes the power GPIO nodes, unlike the tested implementation. |
| `debug` | 4 | Superseded debugging approach; archive before removing. It also drops the local battery module and changes licensing/assets; do not merge it wholesale. |
| `power-btn-combo` | 1 | Old double-tap/tap-dance implementation. The tested branch uses a split-safe dedicated power gesture; archive before removing. |
| `soft-off-plus` | 4 | Earlier power-module integration, superseded by the tested branch. Archive before removing; commits are not ancestors or patch-equivalent. |
| `fix/central-relay-gatt-lifecycle` | 7 | Keep pending review: has a battery PMIC-phandle fix and dependency pins. The external HID module already includes its relay fix, but the firmware branch is not merged. |
| `fix/personal-layout-central-freeze` | 4 | Keep pending hardware testing: pins a newer soft-off module and the controller/HID fixes. The current imported Zephyr already resolves to its controller-fix commit; the soft-off wake changes still need validation. |

The cleanup takes the board/power/battery changes from personal-layout. It keeps
the stock QWERTY keymap and removes the personal keyboard name and host-language
list from the default configuration. It does not merge the two experimental fix
branches. The HID profile pins `ce04bf6`, the current HID module main revision;
that module already contains the merged GATT lifecycle fix.

A full backup was created and verified at
`/home/oleksandr/projects/wafer-before-cleanup-2026-11-10.bundle`. It includes
all nine fetched remote branch tips and complete committed history; the local
cleanup edits are separate working-tree changes.

## Preserving branch history

Before deleting unique branches, create archival tags or a verified Git bundle.
For a local backup containing every currently fetched branch:

```sh
git bundle create ../wafer-before-branch-cleanup.bundle --all
git bundle verify ../wafer-before-branch-cleanup.bundle
```

For each branch to retire, preserve its exact tip as a remote tag first. Example:

```sh
git tag archive/debug origin/debug
git push origin refs/tags/archive/debug
git push origin --delete debug
```

Repeat only for the reviewed branches. Keep both fix branches until their unique
changes have been accepted or deliberately rejected. Deleting a branch does not
remove PCB/CAD files from Git history or reduce the existing repository history.

## Completed cleanup

With the owner's approval, these remote branches were archived as lightweight
`archive/<branch>` tags, verified against their original commit IDs, and deleted:

- `debug2` → `archive/debug2` (`c8154f5`)
- `debug` → `archive/debug` (`2539307`)
- `codex/fix-last-actions-build-error` → `archive/codex/fix-last-actions-build-error` (`3b9d00c`)
- `power-btn-combo` → `archive/power-btn-combo` (`7c00929`)
- `soft-off-plus` → `archive/soft-off-plus` (`641a446`)

Deletion used a lease against each reviewed branch tip to avoid deleting newer
work. `main`, `personal-layout`, and both `fix/` branches remain. To restore an
archived branch, for example:

```sh
git switch -c debug archive/debug
git push -u origin debug
```

The firmware reorganization is separate from this branch cleanup. Commit IDs
above identify the original history, which is preserved in the verified bundle.
The PCB/case path-history rewrite keeps archive tag names but changes affected
tag targets to their firmware-only equivalents; its `commit-map` records the
original-to-rewritten IDs.
