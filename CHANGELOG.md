# Changelog

## [v1.3.0](https://github.com/musqz/fittsmon/releases/tag/v1.3.0) (2026-11-02)

- Changed: `fittsmon-wl` reads `~/.config/fittsmon-wl/fittsmon-wlrc` instead of the X11 `fittsmonrc`. Copy your zones over to keep them.
- Added: `fittsmon-wl` rebuilds its zones and re-reads the config when a monitor is plugged, unplugged or changes mode. Zones follow `--monitor` names, so a selected monitor that comes back gets its zones again. Also frees the shm buffer after attach, which would otherwise leak one per zone per rebuild.
- Docs: README and `fittsmon.1` mention Wayland and `fittsmon-wl`.

## [v1.2.0](https://github.com/musqz/fittsmon/releases/tag/v1.2.0) (2026-10-12)

- Added: `fittsmon-wl`, a Wayland build for wlroots compositors (labwc, sway, niri, Hyprland). Zones are transparent wlr-layer-shell surfaces; it reads the same `~/.config/fittsmon/fittsmonrc` and takes the same `--monitor`, `-l` and `-h` options.
- Changed: `make` builds and `make install` installs both `fittsmon` and `fittsmon-wl`; `make wl` builds only the Wayland binary. The PKGBUILD depends on `wayland`.
- Fix: `fittsmonrc_example` had a `[Left        ]` group with trailing spaces next to the real `[Left]` group. The padded group never matched a position, so it is removed.
- Fix: `fittsmon-wl.o` did not depend on the generated xdg-shell header, so `make -jN` could compile it too early.
- Removed: `fittsmon.1.in`, an unused template that had drifted from `fittsmon.1`.

## [v1.1.1](https://github.com/musqz/fittsmon/releases/tag/v1.1.1) (2026-10-11)

- Fix: screen corners and edges with no command configured now pass clicks through to the window below instead of swallowing them.
- Changed: fittsmon-wl exits with a message when no zone has a usable command.

## [v1.1.0](https://github.com/musqz/fittsmon/releases/tag/v1.1.0) (2025-04-18)

- Changed: renamed from FittsTool to FittsMon for the improved monitor management.
- Changed: the default monitor is used when none is given.
- Fix: several multi-monitor bugs and config handling problems.

## FittsTool 1.0 (2009-03-29)

- Initial release: corner and edge action binding with a configuration file.
