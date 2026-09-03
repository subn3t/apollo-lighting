# Apollo-Inspired Sim Room Lighting Control System — Project Summary

## Overall Goal
Create a high-end tunable-white architectural lighting system for a simulator / utility-room conversion with:

- Extremely high CRI lighting
- Smooth diffuse illumination with minimal hotspots
- Aerospace-inspired tactile controls
- Tunable white lighting (warm ↔ cool)
- ESP32-controlled PWM dimming
- Five independent lighting zones mounted along joists

The overall aesthetic goal is:

> boutique architectural lighting + retro aerospace control panel

NOT:
- RGB gamer room
- theatrical sci-fi cosplay
- industrial factory panel

The desired vibe is:
- clean
- believable
- tactile
- premium
- slightly aerospace-adjacent
- architectural

---

# Room / Physical Scope

## Main room

- ~108.75 sqft floor area
- 5 joist-mounted LED bays (see *LED Channel Layout* below)
- ~15.4 m total installed strip across the 5 bays

## Closet extension

A small under-stairs closet attached to the main room, folded into the Apollo project with first-class-ish treatment. The elevated treatment is partly because the peripheral wall install passes through here, and partly because the closet doubles as a low-stakes testing ground for drywall / epoxy / airless-sprayer skills before touching the main room.

Dimensions & profile:
- 122" long × 36" wide (~30.5 sqft footprint)
- Odd Harry-Potter-under-stairs ceiling profile (sloping down along the length)
- Only the front **31"** of the 122" length is full-height — this is the section under the exposed ceiling joists where LED strips can mount
- Only ~**80"** of the 122" length is really usable floor space; the remaining ~42" is only usable as storage for small items that fit under the low end of the stairs

Finish:
- Same paint finish as the main Apollo room
- Same epoxy floor as the main Apollo room
- Door already built and installed

Lighting:
- Two exposed ceiling joists in the full-height section (running perpendicular to the 122" length / x-axis)
- Each joist fits **32"** of usable FilmGrade Hybrid strip → **64" total** (~1.63 m), cut from leftover main-room reel stock (requires pad soldering on the cut end — the strip's 4" cut-line pads are large and forgiving)
- Realistic output at full both-channels: **~2,200 lm ≈ ~110 lm/sqft** over the useful 80" × 36" floor — meaningfully dimmer than the main room's ~190 lm/sqft ceiling, correct for a companion space rather than a competing one
- Strip mounted in the same aluminum channels + opal diffuser as the main room, both for the visual match and for thermal management (~30 W in a small enclosed volume)

Control / activation (deliberately standalone — not driven by the main ESP32 / panels):
- **Magnetic reed switch on the door** (normally-closed — circuit closes when magnet is absent). Door open → light on, door closed → light off. Fridge-style behavior; zero interaction, no visible wall control, no wall real estate consumed.
- **Dedicated 24V DC wall brick** (~48 W, well over the ~30 W max load) powers the strip locally inside the closet. No PWM, no dimming, no microcontroller.
- **Both warm and cool channel negatives tied together** on the strip side and switched via the reed switch. When lit, both channels are always at full → ~4000K mix, matching the main room's happy-place color temperature.
- **Inline AC-side rocker switch** between wall outlet and brick as a manual "disable entirely" master kill.
- **Reed switch current-rating gotcha:** the strip draws ~1.25 A at full (64" × 230 mA/ft across both channels), so the reed switch must be rated **≥2 A DC**. Cheap alarm-system reeds are often 0.5–1 A rated and would burn under this load. Either buy a ≥2 A door-contact switch directly, or drive a small 24V relay coil from a cheaper reed and switch the strip through the relay contacts.

Rationale for keeping the closet off the main control system:
- Scope-creep firewall — a closet doesn't need a Concord Aerospace panel to answer "is someone in the closet?"
- Zero-interaction feel matches the space's actual usage (grab something, leave)
- Independent power path means the closet works even when the main enclosure is off (useful during main-system service or when the panel switches are in an unknown state)

---

# Lighting System Architecture

## LED Strips
Using:

Waveform Lighting FilmGrade Hybrid Tunable White LED Strips

Quantity:
- 5 reels

Characteristics:
- High CRI (~95)
- Tunable white (tungsten 3200K + daylight 6500K LEDs co-mounted)
- Premium architectural-quality lighting
- 1500 lumens/meter total output (both channels combined; 750 lm/m per channel)
- 24V system
- 2.8 W/ft (≈9.2 W/m) per channel — so 5.6 W/ft (≈18.4 W/m) at full blast with both channels on
- Cuttable every 100 mm (4")
- Max single run from one power injection: 10 m (we're well under this — each bay ~3 m)
- Source: datasheet `FilmGrade_HYBRID_WHITE_LED_Strip_for_Film_Photography_3002.HY_Specification_Sheet.pdf` (PN 3002.HY, Rev 1.01)

Installed length estimate:
- ~15.4 meters total (5 bays × ~3.08 m each)

Total purchased:
- 25 meters total (5 × 5m reels)

Reason for overbuy:
- spare material
- future additions
- mistakes
- consistent appearance

---

# Color Temperature Philosophy

User strongly prefers:
- 4000K–4200K operating range

User observations:
- 4000K appears perceptually brightest
- 5000K feels too cold / unnatural
- 6500K feels “RGB blue” / unpleasant
- 2700–2900K feels cozy/home-like

Lighting philosophy:
- neutral-bright architectural white
- not clinical hospital blue
- not warm residential amber all the time

The system should support:
- daylight-ish operation
- warm evening ambiance
- dim standby mode

---

# LED Channel Layout

There are:
- 5 physical joist-mounted lighting strips

Each strip contains:
- warm LED channel
- cool LED channel

Total PWM channels required:
- 10

Calculation:
- 5 strips × 2 channels each = 10 PWM outputs

---

# Aluminum Channels / Diffusers

Using:
- Deep aluminum LED channels with milky/opalescent diffuser

Reasoning:
- minimize visible LED hotspots
- create smooth continuous light
- increase LED-to-diffuser spacing
- premium architectural appearance

Purchased:
- 10-pack of 6.6ft (2m) deep “spotless” channels
- ~66 ft total

This exceeds estimated required length (~50 ft) and intentionally includes spare material.

---

# Power Supply

Using:
- Mean Well LRS-450-24

Specifications:
- 24V
- 450W
- ~18.8A max output

Reason chosen:
- industry-standard
- reliable
- inexpensive
- appropriate for LED system

Sizing (verified against datasheet):
- 15.4 m installed × 18.4 W/m (both channels at full) = ~283 W peak load
- DC side: ~11.8 A @ 24 V (vs PSU's 18.8 A rated max)
- AC side: ~2.6 A @ 120 V (vs the 5 A inline fuse)
- PSU loaded to ~63% of rated capacity at absolute full blast — sweet spot for switching PSU efficiency and lifespan
- A 350 W PSU (LRS-350-24) would have been 81% loaded at full blast — workable but tight; the 450 W gives proper headroom for indefinite full-output operation

Important understanding:
- individual LED runs do NOT carry full PSU power
- power is distributed across many channels

---

# Wire Gauge Philosophy

## LED Power Wiring
Recommended:
- 18 AWG stranded copper

Usage:
- strip home runs
- internal 24V power distribution

Cable concept:
- one 18/3 cable per strip

Each 18/3 run carries:
- shared +24V
- warm negative
- cool negative

Reasoning:
- flexible
- easy to terminate
- appropriate current handling
- manageable inside enclosure

## Low-current signal wiring
Recommended:
- 22–24 AWG stranded wire

Used for:
- switch signals
- ESP32 GPIO
- MOSFET control signals

---

# Shared +24V Architecture

System uses:
- common/shared +24V rail

Concept:
- all LED channels receive common +24V
- MOSFET boards switch the negative side

This is low-side PWM switching.

MOSFET outputs switch:
- warm negative
- cool negative

ESP32 controls MOSFET gates with PWM signals.

---

# Controller Architecture

## Microcontroller
Using:
- ESP32 development board
- 38-pin variant (NodeMCU-32S / DOIT DevKit V1 form factor), generic `esp32dev`

Purchased:
- matched kit including:
  - ESP32 boards
  - matching screw-terminal breakout adapters

History:
- the project initially targeted a 30-pin AITRIP board, then moved to the 38-pin
  board to expose enough GPIOs to wire all 9 panel switches/button + the I²C bus
  directly, with no I/O expander
- the 30-pin AITRIP board is kept as a backup / breadboard unit

The breakout boards expose GPIO via screw terminals.

---

# MOSFET Power Stage

Using:
- 4-channel MOSFET PWM trigger boards

Purchased:
- 3 boards total

Each board provides:
- 4 PWM channels

Total available:
- 12 channels

Required:
- 10 channels

Result:
- 2 spare channels available

MOSFET boards are used as:
- PWM power switching stage
- “muscle” between ESP32 and LEDs

ESP32 outputs:
- low-current logic signals

MOSFET boards switch:
- real 24V LED power

---

# Control Enclosure

Using:
- VEVOR 12" × 12" × 6" steel enclosure
- includes backplate

Reasoning:
- compact
- enough room for:
  - PSU
  - ESP32
  - MOSFETs
  - moderate wire routing

The enclosure interior does NOT need to be industrial-art-piece quality.

Goal:
- functional
- reasonably tidy
- practical

NOT:
- aerospace-certified control cabinet

---

# Electrical Safety Philosophy

Important distinction:

## AC side (code-sensitive)
This includes:
- wall AC input
- PSU mains input
- grounding

Must:
- properly ground enclosure
- use proper AC entry / strain relief
- avoid exposed mains wiring

## Low-voltage side
This includes:
- 24V LED power
- ESP32
- PWM wiring
- switches

This is treated more like:
- appliance internals
- low-voltage electronics

Not residential branch wiring.

---

# Apollo Control Panels

## Main Switch Panel
Panel type:
- Concord Aerospace “Apollo Master Alarm Panel”

Purpose:
- individual strip enable control
- standby mode
- master reset button

Labeling:

PANEL NAME:
PRIMARY ILLUMINATION

BUTTON NAME:
MASTER RESET

Switch layout:

FIELD 1: ENABLE
FIELD 4: BAY 1

FIELD 2: ENABLE
FIELD 5: BAY 2

FIELD 3: ENABLE
FIELD 6: BAY 3

FIELD 7: ENABLE
FIELD 10: BAY 4

FIELD 8: ENABLE
FIELD 11: BAY 5

FIELD 9: STANDBY
FIELD 12: AUX

Interpretation:
- each switch “enables” one lighting bay
- sixth switch activates standby/aux mode

## Standby Mode Philosophy
Intended behavior:
- dim warm ambient architectural glow
- very low brightness
- atmospheric “facility resting state”

Possible implementation:
- all strips forced to:
  - low warm output
  - near-zero cool output

Purpose:
- nighttime ambiance
- subtle navigation light
- simulator atmosphere

---

# Warm Bias Panel

Using:
- Concord “Ludicrous Speed” panel

Purpose:
- global warm-channel intensity control

Labels:

PANEL NAME:
PRIMARY ILLUMINATION

DIAL NAME:
WARM BIAS

FIELD 1:
DAYLIGHT

FIELD 2:
INCAND

TOGGLE NAME:
WARM ENABLE

Important semantic philosophy:
- labels describe resulting room feel
- not raw electronics state

Meaning:
- more warm contribution pushes room toward incandescent warmth
- less warm contribution trends toward daylight appearance

---

# Cool Bias Panel

Using:
- second Concord “Ludicrous Speed” panel

Purpose:
- global cool-channel intensity control

Labels:

PANEL NAME:
PRIMARY ILLUMINATION

DIAL NAME:
COOL BIAS

FIELD 1:
INCAND

FIELD 2:
DAYLIGHT

TOGGLE NAME:
COOL ENABLE

---

# Interface Philosophy

System intentionally avoids:
- RGB gamer aesthetics
- software-only interaction
- touchscreens

Focus is on:
- tactile controls
- environmental feel
- believable aerospace flavor
- architectural interaction design

Controls should feel:
- premium
- slightly industrial
- functional
- not parody/cosplay

---

# Remaining Major Purchases / Tasks

Still needed:

## Wiring
- 18/3 stranded cable
- 22–24 AWG hookup wire

## Connectors / Termination
- ferrule kit
- ferrule crimper

## AC Power Entry
Direct Romex (NM-B) feed — decided:
- the enclosure is recessed into the wall; building AC is run to it as NM-B cable
- NM-B lands on the PSU AC terminals through a proper NM cable clamp at the knockout
- metal enclosure bonded to the AC ground

See `BACKPLATE.html` (*AC Entry & In-Wall Rough-In*) for routing and securing.

## Assembly
Need to:
- mount PSU
- mount ESP32/breakouts
- mount MOSFET boards
- route LED wiring
- cut panel openings
- install Apollo controls
- write ESP32 firmware

---

# Firmware / Software Intent

ESP32 firmware responsibilities:

- read toggle switches
- read potentiometer values
- output PWM dimming signals
- implement standby mode
- implement master reset behavior
- combine warm/cool bias logic

Programming approach:
- likely Arduino framework
- user plans to use Claude Code heavily during development

---

# Current Total Project Spend (approx)

Approx invested so far:

~$1,627 USD

Includes:
- LED strips
- channels/diffusers
- control panels
- PSU
- enclosure
- ESP32 hardware
- MOSFET boards

Not yet including:
- wire
- ferrules
- miscellaneous assembly hardware

