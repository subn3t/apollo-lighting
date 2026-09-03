# Apollo Sim Room Lighting Control

ESP32 firmware + electronics for a 5-zone, tunable-white architectural LED system driven by Apollo-style aerospace control panels. Installed in a ~108.75 sqft simulator / utility room; a lighting extension into the attached under-stairs closet is planned (see Open items). Companion docs: `apollo_lighting_control_system_summary.md` (design context — read before non-trivial changes), `BACKPLATE.html` (the maintained, scaled enclosure layout, wiring flow, and build guide — source of truth for component positions, wiring, mounting, and drilling), and the LED strip datasheet `FilmGrade_HYBRID_WHITE_LED_Strip_for_Film_Photography_3002.HY_Specification_Sheet.pdf` (PN 3002.HY, Rev 1.01). (`WIRING.md` is retired — superseded by `BACKPLATE.html`.)

## Build / dev workflow

- **Framework:** PlatformIO + Arduino-ESP32 core. `platformio.ini` is the source of truth for board, framework, and library deps.
- **Board:** 38-pin ESP32 dev board (NodeMCU-32S / DOIT DevKit V1 form factor), generic `esp32dev`. Same WROOM-32 chip as the 30-pin variant — the project initially targeted a 30-pin AITRIP board but moved to 38-pin to gain enough GPIOs for direct wiring of all 9 panel switches/button + the I²C display, eliminating the need for an I/O expander. The 30-pin AITRIP is kept as a backup/breadboard unit.
- **Hardware is connected** to this PC over USB-C during development. Prefer real-hardware verification over pure reasoning when possible.
- **Common commands:**
  - `pio run` — build
  - `pio run -t upload` — flash the connected board
  - `pio device monitor` — serial monitor at 115200 baud
- **Arduino-ESP32 v2.x is currently installed.** Use the v2.x LEDC API: `ledcSetup(channel, freq, res)` + `ledcAttachPin(pin, channel)` + `ledcWrite(channel, duty)` — channel-based, not the pin-based v3.x API (`ledcAttach` / `ledcWrite(pin, duty)`).

## Aesthetic intent (load-bearing — not decoration)

Design constraints, not flavor text:

- **Aim for:** boutique architectural lighting + retro aerospace control panel. Tactile, premium, believable.
- **Avoid:** RGB gamer aesthetic, sci-fi cosplay, industrial-factory look, software-only or touchscreen interaction.
- Labels describe *room feel* (e.g. `INCAND` / `DAYLIGHT`), not raw electronics state.

If a choice would make this read as a gamer rig or a parody, it's wrong even if technically correct.

## Color temperature

- Operating range: **4000K–4200K** is the user's strong preference — this is where defaults should land.
- 5000K reads as too cold; 6500K is "RGB blue" and unpleasant; **3200K (tungsten — the strip's warm endpoint)** is the warmest reachable, used for standby / cozy moods. True candle-warm 2700K is not physically achievable with the FilmGrade Hybrid strip.

## Channel layout

- 5 strips × (warm + cool) = **10 PWM channels**.
- 3× 4-channel MOSFET boards = 12 channels available, 2 spare.
- Shared **+24V rail**; MOSFETs do **low-side** PWM on warm- and cool- per strip.

## Control surfaces

Three Concord Aerospace panels + two door-mounted readouts (CCT and lumens):

1. **Apollo Master Alarm Panel** — `PRIMARY ILLUMINATION`
   - 5× `ENABLE` toggles (one per BAY 1–5)
   - 1× `STANDBY` toggle
   - `MASTER RESET` button
2. **Ludicrous Speed #1** — `WARM BIAS` dial (`DAYLIGHT` ↔ `INCAND`) + `WARM ENABLE` toggle. Global warm-channel intensity.
3. **Ludicrous Speed #2** — `COOL BIAS` dial (`INCAND` ↔ `DAYLIGHT`) + `COOL ENABLE` toggle. Global cool-channel intensity.
4. **CCT readout** — 4-digit blue 7-segment I2C display (HT16K33 backpack, address `0x70`) mounted on the door, with a permanent screen-printed/etched `K` label on the bezel. Shows the effective color temperature being mixed (range 3200K–6500K per the strip's physical endpoints, defaults landing in the 4000–4200K band). Aesthetic target: DSKY-adjacent glowing segments.
5. **Lumens readout** — a second 4-digit blue 7-segment I2C display (HT16K33 backpack, address `0x71`), door-mounted beside the CCT readout, with a permanent screen-printed/etched `lm` label on the bezel. Shows total system light output (system max ≈23,100 lm at full blast). 1–9999 lm: a plain integer (leading zeros blanked). 10,000 lm and up: a compressed scientific-notation form — the value with a lit decimal point standing in for ×10,000 (e.g. 12,346 lm → `1.234`, 23,100 lm → `2.310`). The two displays match visually and read as a paired instrument cluster.

When `master_on` is false, both displays are blank — the lit displays are the "system on" indicator alongside the button LED.

### Standby mode

Low warm output, near-zero cool output, across enabled strips. Mood: dim ambient architectural glow, "facility at rest." Not off, not bright.

### Master Reset

Functions as the system's master on/off. The button is momentary; firmware edge-detects presses to toggle a `master_on` boolean. When `master_on` is false, all PWM outputs are forced to 0 regardless of panel state. When true, the panel toggles/dials are the source of truth — there is no internal state to "reset," because the physical switches are the source of truth. The button's integrated 5–12V LED is illuminated when `master_on` is false ("press here to wake up") and dark when on.

## Firmware responsibilities

- Read 9 digital inputs (5 bay enables + standby + `WARM ENABLE` + `COOL ENABLE` + `MASTER RESET` button) directly on ESP32 GPIOs using `INPUT_PULLUP`. Switch-to-GND wiring (closed = LOW). Edge-detect the momentary `MASTER RESET` to toggle `master_on`.
- Read 2 potentiometers (warm bias, cool bias) directly on ESP32 ADC1.
- Drive 10 PWM outputs (5 warm + 5 cool) low-side via MOSFET boards.
- Drive `MASTER RESET` button's integrated LED (lit when system is off, dark when on).
- Drive both 4-digit readouts (two HT16K33s over I2C, `0x70`/`0x71`). Compute effective CCT from the warm/cool mix for the CCT display and total system lumens for the lumens display; blank both when `master_on` is false.
- Combine bias dials with per-bay enables to compute each channel's duty cycle.
- Implement standby and master on/off behavior.

## Responsiveness / feel

The panel must feel like a tactile analog instrument — knobs that move the light *instantly* and smoothly, displays that track without lag. It must never feel like a Bluetooth/IoT dimmer. This is a firmware constraint, not just a hope: the hardware is far faster than human perception (the pot→PWM loop runs sub-millisecond against a ~100 ms "feels instant" threshold), so the feel is entirely down to how the firmware is written.

- **Keep the control loop non-blocking.** No `delay()` in `loop()`. The pot→PWM path must run every iteration, uninterrupted. The display write goes on a timer (below), never inline.
- **The real risk is jitter, not lag.** The ESP32 SAR ADC is noisy — a raw `analogRead` on a held-still pot wobbles by several to tens of counts. Mapped straight to PWM that shimmers the light; fed to a display it makes the digits dance. Mitigate on both sides:
  - **Hardware:** a ~0.1 µF cap from each pot wiper to GND, at the ESP32 ADC pin (≈1 ms RC with a 10 kΩ pot) — kills high-frequency noise and umbilical pickup.
  - **Firmware:** light smoothing on the ADC reads (exponential moving average or short rolling average). Keep it *light* — just enough to kill jitter. Over-filtering is the one thing that adds perceptible lag; that's the dial to tune.
- **Display refresh:** update both HT16K33s on a fixed ~20–30 Hz timer using the smoothed values, and only when the displayed value actually changed. Never redraw every loop. Result: quick, lag-free, no digit flicker.
- **Perceptual dimming curve.** The eye is logarithmic — a linear pot→duty map feels lopsided. Map pot position to duty through a gamma/perceptual curve so equal knob rotation gives equal perceived brightness change. Clamp the knob extremes to true-off / true-full so the ends are reliable.
- **Future WiFi/OTA must not compromise this.** `loop()` is on core 1 and the radio on core 0, which mostly insulates the control loop — but keep any WiFi/OTA work non-blocking so it can never stall the pot→PWM path. The panel feels instant regardless of what the radio is doing.

## I2C bus

Two devices, both door-mounted **HT16K33 4-digit 7-seg displays**: the CCT readout at `0x70` and the lumens readout at `0x71`. SDA = GPIO 21, SCL = GPIO 22. The second display's address is set by bridging the `A0` solder jumper on its backpack — do this before mounting and label it.

Adding the second display costs **zero GPIOs** — I²C is a bus, so both displays share the same two pins. This is the same "expand over I²C" path the pin map reserves for all future additions.

Each backpack carries its own I²C pull-ups. With two backpacks the pull-ups sit in parallel (~5 kΩ effective) — still within spec for this short, standard-mode 100 kHz bus; no need to remove either set. The 4-wire bus (SDA / SCL / 3V3 / GND) crosses the door hinge alongside the switch/pot umbilical; the two displays are daisy-chained on the door, so only one 4-wire run crosses the hinge.

### CCT calculation

`CCT_warm = 3200K`, `CCT_cool = 6500K` — these are the FilmGrade Hybrid strip's physical endpoints (tungsten + daylight LEDs co-mounted on the strip, per datasheet PN 3002.HY). Recalibrate if measured CCT differs.

```
warm_eff = warm_bias * warm_enable
cool_eff = cool_bias * cool_enable
if warm_eff + cool_eff == 0: display blank
else: CCT = CCT_warm + (cool_eff / (warm_eff + cool_eff)) * (CCT_cool - CCT_warm)
```

This is per-channel-mix CCT, not per-bay; the displayed value is what a single fully-enabled bay would put out.

### Lumens calculation

Unlike CCT (a per-channel-mix figure), the lumens readout is a **whole-system total** — the sum of what all five bays are actually emitting.

```
LUMENS_FULL_PER_CHANNEL = 2310   # derived: 15.4 m installed × 750 lm/m per color ÷ 5 strips
total_lumens = 0
for each of the 10 PWM channels:
    total_lumens += (duty / duty_max) * LUMENS_FULL_PER_CHANNEL
```

Derivation: the FilmGrade Hybrid datasheet specifies 1500 lm/m total = 750 lm/m per channel. Installed strip is 15.4 m across 5 bays (avg ~3.08 m/bay). Per-channel full-output lumens = 3.08 m × 750 lm/m ≈ 2310 lm. Whole-system max with all 10 channels at full = ~23,100 lm.

`duty` is the channel's final computed duty cycle — it already folds in bias, per-bay enable, warm/cool enable, standby, and `master_on`, so a disabled or off channel contributes 0.

Display formatting:

```
if (not master_on) or total_lumens == 0:  display blank
elif total_lumens < 10000:                 show integer, no decimal point   # 1 .. 9999
else:                                       show floor(total_lumens / 10) as D.DDD,
                                            decimal point lit                # 12,346 -> 1.234
                                                                             # 23,100 -> 2.310 (system max at full blast)
```

`LUMENS_FULL_PER_CHANNEL = 2310` is derived from datasheet specs (1500 lm/m total ÷ 2 channels × ~3.08 m/bay) but is still nominal — verify against a light meter once the strips are up and tune. The datasheet figure assumes a fresh strip at 24V nominal with no voltage drop; real installs typically read 5–15% lower.

## Wiring conventions

- **LED power runs:** 18/3 stranded — shared +24V, warm-, cool- per strip.
- **Signal wiring:** 22–24 AWG stranded for switches, GPIO, MOSFET gates.
- **AC side is code-sensitive** (proper grounding, strain relief, no exposed mains). The AC feed is direct Romex (NM-B) — the enclosure is recessed into the wall; see `BACKPLATE.html` (*AC Entry & In-Wall Rough-In*) for routing and securing. Low-voltage side is treated as appliance internals.

## Pin map

Designed with **WiFi/OTA on the table for a future phase** — both pots are on ADC1 so they keep working when WiFi is up. GPIO 12 and 2 are used for PWM outputs only; both are safe because PWM idles LOW, which matches their boot-time strapping requirement (and the MOSFET gate pull-down resistors hold them LOW at boot). GPIO 15 is used for a low-stakes digital output (button LED); its only boot-time effect is suppressing serial bootloader chatter, which is harmless or even desirable.

GPIO 0 and GPIO 5 are strapping pins that the 38-pin board exposes — both are usable here with specific input-style assignments noted below.

### Analog inputs

| GPIO | Role | Notes |
|------|------|-------|
| 36 (VP) | `WARM BIAS` pot | ADC1, input-only · 0.1 µF wiper→GND cap (see Responsiveness / feel) |
| 39 (VN) | `COOL BIAS` pot | ADC1, input-only · 0.1 µF wiper→GND cap (see Responsiveness / feel) |

### Digital inputs (direct GPIO, all `INPUT_PULLUP`, switch-to-GND, closed = LOW)

| GPIO | Role | Notes |
|------|------|-------|
| 34 | `BAY 1 ENABLE` toggle | input-only |
| 35 | `BAY 2 ENABLE` toggle | input-only |
| 32 | `BAY 3 ENABLE` toggle | |
| 33 | `BAY 4 ENABLE` toggle | |
| 27 | `BAY 5 ENABLE` toggle | |
| 26 | `STANDBY` toggle | |
| 23 | `WARM ENABLE` toggle | |
|  5 | `COOL ENABLE` toggle | Strapping pin (samples HIGH at boot for normal flash boot). Open switch reads HIGH via pull-up → fine. Switch closed at boot pulls it LOW, which doesn't break ESP32 boot in practice (only affects SDIO timing config). |
|  0 | `MASTER RESET` button (momentary) | Strapping pin tied to dev board's BOOT button. The panel button is in parallel with BOOT. Boot-time risk only if the button is held DURING power-on (puts ESP32 into download mode) — not an issue in normal use since the button is normally open. |

### I2C bus (displays only)

| GPIO | Role |
|------|------|
| 21 | I2C SDA |
| 22 | I2C SCL |

Two devices on the bus: HT16K33 CCT display (`0x70`) and HT16K33 lumens display (`0x71`). The second display adds no GPIO cost.

### PWM outputs (to MOSFET gates on the 24V power MOSFET bank)

| GPIO | Channel |
|------|---------|
| 14 | Strip 1 WARM |
| 12 | Strip 1 COOL |
| 13 | Strip 2 WARM |
|  4 | Strip 2 COOL |
| 16 | Strip 3 WARM |
| 17 | Strip 3 COOL |
| 18 | Strip 4 WARM |
| 19 | Strip 4 COOL |
| 25 | Strip 5 WARM |
|  2 | Strip 5 COOL — also onboard LED on this dev board (mirrors this channel cosmetically) |

### Status outputs (NOT on the 24V MOSFET bank)

| GPIO | Channel | Notes |
|------|---------|-------|
| 15 | `MASTER RESET` integrated LED | 5–12V lamp; switched via a MOSFET driver module powered from 5V. Do **not** connect to the 24V MOSFET bank — that voltage would destroy the LED. |

### Free / available

All 24 usable GPIOs are allocated — zero spare on the ESP32 itself. For future expansion (sensors, additional outputs, etc.), use an I²C-bus device added on the existing SDA/SCL pins — the same way the lumens display was added at no GPIO cost. The two HT16K33s sit at `0x70`/`0x71`, leaving the rest of the I²C address space clear of the common expander/sensor addresses.

### Reserved / do not use

- **GPIO 1, 3** — USB serial TX/RX (would break the serial monitor).
- **GPIO 6–11** — onboard flash.

## Open items

- During prototyping, the MASTER RESET button LED runs off the ESP32 dev board's 5V pin (USB-powered). For the final build, both ESP32 and button LED can share a small 24V→5V buck converter sitting on the 24V rail.
- AC feed is direct Romex (NM-B) into a clamped knockout — see `BACKPLATE.html` (*AC Entry & In-Wall Rough-In*). Remaining check: confirm the supplying circuit is sized for the build (≤ ~4 A at 120 V; a 15 A circuit is ample).
- Button LED motion: steady-on vs slow pulse vs fade — defer until prototype.
- CCT and lumens display behavior in standby and at zero output (blank vs `----` vs `STBY`) — defer until the displays arrive.
- CCT constants (3200K / 6500K) and `LUMENS_FULL_PER_CHANNEL` (2310 lm) are derived from datasheet specs (PN 3002.HY) × installed length, but still nominal — verify CCT with a meter/visual reference and lumens with a light meter once strips are installed; tune if needed. Datasheet figures assume nominal 24V with no voltage drop; real installs typically read 5–15% lower on lumens.
- Display brightness — each HT16K33 has 16 brightness levels; pick one that reads as "instrument lit" rather than "screen on," and match both displays.
- Booting with a switch closed on GPIO 5 has no real-world effect, but document it during commissioning so future-you doesn't get confused if a SDIO-timing config message ever appears in serial output.
- If a user holds the panel `MASTER RESET` button during power-on, the ESP32 enters download mode (GPIO 0 pulled LOW at boot). Not a normal scenario, but worth a small note on the panel or in the docs so it doesn't happen by accident during a service event.
- **Closet extension:** an under-stairs closet attached to the main room (122" × 36", ~30.5 sqft footprint, ~20 sqft usable floor) is receiving first-class-ish Apollo treatment — same paint finish, same epoxy floor. Odd Harry-Potter-under-stairs ceiling profile: only the front 31" of the 122" length is full-height. Two exposed ceiling joists in that section each fit 32" of usable LED strip = 64" total (~1.63 m, ~1.25 A at full, cut from leftover main-room reel stock). Activation is deliberately standalone — a magnetic reed switch on the door drives the strip directly from a dedicated 24V wall brick (no dimming, no PWM, no ESP32 involvement). Both channels tied together → ~4000K mix at full whenever the door is open. Manual AC-side rocker between outlet and brick as a master kill. See `apollo_lighting_control_system_summary.md` (*Closet extension*) for the full architecture and the reed-switch current-rating gotcha.
