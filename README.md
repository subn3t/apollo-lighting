# Apollo Sim Room Lighting

A 5-zone, tunable-white architectural LED system driven by Apollo-style aerospace control panels. ESP32 firmware, custom electronics, and a physical install in a ~108.75 sqft simulator / utility room, with a standalone extension into the adjacent under-stairs closet.

The goal isn't RGB flash or software dimming — it's **boutique architectural lighting operated by tactile analog instruments**, so the room reads as a piece of considered space rather than a gamer rig.

## The install

- **5 bays** × warm+cool strip = **10 low-side PWM channels** across three 4-ch MOSFET boards on a shared 24 V rail.
- **10 physical controls** on three Concord Aerospace panels: an Apollo Master Alarm panel (5 bay enables, Standby, Master Reset) and two Ludicrous Speed panels (warm + cool bias pots and per-side enables).
- **Two door-mounted 4-digit blue 7-seg readouts** (HT16K33s on I²C): effective CCT and total system lumens — the instrument-cluster "system on" indicator.
- **FilmGrade Hybrid tunable-white strip** (PN 3002.HY), tungsten (3200 K) + daylight (6500 K) LEDs co-mounted. Design center is 4000–4200 K.
- **Closet extension** (under-stairs, standalone): reed switch on the door drives a 24 V relay that gates a dedicated brick. No microcontroller — fridge-light behavior, always ~4000 K when on.

Interim install landed on 2026-07-03 and exceeded expectations. Firmware is early (a slice-2 skeleton in `src/main.cpp`); the full control loop is next.

## Documentation

- [`CLAUDE.md`](CLAUDE.md) — canonical spec: aesthetic intent, channel layout, pin map, firmware responsibilities, responsiveness constraints, CCT/lumens math, open items.
- [`apollo_lighting_control_system_summary.md`](apollo_lighting_control_system_summary.md) — design rationale and context; read before non-trivial changes.
- [`BACKPLATE.html`](BACKPLATE.html) — scaled enclosure layout, wiring flow, and build guide for the main-room control box. Source of truth for component positions, wiring, mounting, and drilling.
- [`CLOSET.html`](CLOSET.html) — wiring diagram and bench-test procedure for the standalone closet extension.
- [`simulator.html`](simulator.html) — an interactive control-surface preview used during panel and firmware design.

## Hardware

- **MCU:** ESP32 dev board (38-pin WROOM-32, DOIT DevKit V1 form factor). 24 of 24 usable GPIOs allocated; future expansion goes over I²C on the existing bus.
- **Power:** 24 V DC (LRS-350 / LRS-450 class supply — see PDFs). Low-side PWM via three 4-channel MOSFET boards.
- **Displays:** two HT16K33 4-digit 7-segment blue backpacks at `0x70` (CCT) and `0x71` (lumens), sharing SDA/SCL across the door hinge.
- **Panels:** three Concord Aerospace panels (one Master Alarm, two Ludicrous Speed).
- **Strip:** ~15.4 m of FilmGrade Hybrid PN 3002.HY across five bays; ~1.63 m of the same reel drives the closet.

## Firmware

PlatformIO + Arduino-ESP32 v2.x core, targeting `esp32dev`.

```
pio run              # build
pio run -t upload    # flash
pio device monitor   # serial at 115200
```

`src/` holds the main firmware. `tools/` contains small standalone sketches used during bring-up (`i2c-scanner`, `channel-walk`, `all-on`, `display-demo`).

## Repo layout

```
src/                             main firmware
tools/                           bring-up sketches (i2c scan, channel walk, all-on, display demo)
platformio.ini                   board / framework / library config
CLAUDE.md                        canonical spec — read first
apollo_lighting_control_system_summary.md   design rationale
BACKPLATE.html                   main-room enclosure & wiring build guide
CLOSET.html                      closet-extension wiring & bench test
simulator.html                   control-surface preview
*.pdf                            strip and supply datasheets
*.png                            reference photos, component shots, install snapshots
```

## Status

- ✅ Main-room panels wired and installed
- ✅ 24 V rail + MOSFET banks bench-tested
- ✅ Interim lighting install (2026-07-03)
- ✅ Closet extension design + bench-test wiring diagram
- 🔨 Main firmware: skeleton committed; full control loop, standby, CCT/lumens display in progress
- 🔨 Closet extension: bench validation next, then final install
