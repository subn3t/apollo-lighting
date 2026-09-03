// Channel walk diagnostic.
//
// Marches one PWM channel at a time to full duty, all others hard-off. Cycles
// through every strip warm/cool pair in the documented order so you can confirm
// that each ESP32 GPIO drives the correct MOSFET output and the correct strip.
//
// What to watch for:
//   - Strips light in order: S1 warm, S1 cool, S2 warm, S2 cool, ... S5 cool.
//     Wrong order  -> firmware pin map vs. wiring mismatch.
//     Wrong strip  -> swapped MOSFET-board outputs or strip- wires.
//   - Each channel goes fully bright then fully dark, no residual glow.
//     Glow-when-off  -> missing or weak gate pull-down resistor.
//   - When Strip 5 COOL lights, the dev board's onboard blue LED also lights.
//     This is expected: GPIO 2 is shared with the onboard LED.
//
// Flash:    pio run -d tools/channel-walk -t upload
// Monitor:  pio device monitor -d tools/channel-walk

#include <Arduino.h>

struct Channel {
  const char* label;
  uint8_t     gpio;
};

// Walk order matches CLAUDE.md "PWM outputs" table top-to-bottom.
constexpr Channel kChannels[] = {
  {"Strip 1 WARM", 14},
  {"Strip 1 COOL", 12},
  {"Strip 2 WARM", 13},
  {"Strip 2 COOL",  4},
  {"Strip 3 WARM", 16},
  {"Strip 3 COOL", 17},
  {"Strip 4 WARM", 18},
  {"Strip 4 COOL", 19},
  {"Strip 5 WARM", 25},
  {"Strip 5 COOL",  2},  // also onboard LED on this dev board
};
constexpr size_t kChannelCount = sizeof(kChannels) / sizeof(kChannels[0]);

constexpr uint32_t kPwmFrequencyHz    = 5000;
constexpr uint8_t  kPwmResolutionBits = 12;
constexpr uint32_t kDutyFull          = (1u << kPwmResolutionBits) - 1;

constexpr uint32_t kOnMs       = 5000;  // how long each channel stays at full
constexpr uint32_t kGapMs      =  250;  // all-off gap between channels
constexpr uint32_t kCycleEndMs = 1000;  // longer all-off pause at end of cycle

static void allOff() {
  for (size_t i = 0; i < kChannelCount; ++i) {
    ledcWrite(i, 0);
  }
}

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=== channel-walk: one PWM channel at a time, full duty ===");
  Serial.printf("PWM: %u Hz, %u-bit (duty 0..%u)\n",
                kPwmFrequencyHz, kPwmResolutionBits, kDutyFull);
  Serial.printf("Each channel ON for %lu ms, %lu ms gap between, %lu ms at cycle end.\n",
                (unsigned long)kOnMs, (unsigned long)kGapMs,
                (unsigned long)kCycleEndMs);
  Serial.println();

  // One LEDC channel per pin (channels 0..9). 12-bit, 5 kHz to match main fw.
  for (size_t i = 0; i < kChannelCount; ++i) {
    ledcSetup(i, kPwmFrequencyHz, kPwmResolutionBits);
    ledcAttachPin(kChannels[i].gpio, i);
    ledcWrite(i, 0);
  }
}

void loop() {
  for (size_t i = 0; i < kChannelCount; ++i) {
    allOff();
    Serial.printf("[%u/%u] ON  GPIO %2u  %s\n",
                  (unsigned)(i + 1), (unsigned)kChannelCount,
                  kChannels[i].gpio, kChannels[i].label);
    ledcWrite(i, kDutyFull);
    delay(kOnMs);

    ledcWrite(i, 0);
    delay(kGapMs);
  }
  Serial.println("--- cycle complete, pausing ---");
  allOff();
  delay(kCycleEndMs);
}
