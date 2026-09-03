// All-on diagnostic.
//
// Drives every PWM channel to full duty and holds it there. No fading, no
// switching — just 10 GPIOs steady HIGH so you can probe any/all of them
// with a meter or confirm every strip lights at once.
//
// Expected readings (relative to GND):
//   ESP32 GPIO / MOSFET-board input screw : steady ~3.3 V DC
//   MOSFET-board strip- output screw      : ~0 V (MOSFET fully on)
//   LED strip (+24V to strip-)            : ~24 V across the strip
//   Onboard blue LED                      : solid on (GPIO 2 = Strip 5 COOL)
//
// Flash:    pio run -d tools/all-on -t upload
// Monitor:  pio device monitor -d tools/all-on

#include <Arduino.h>

struct Channel {
  const char* label;
  uint8_t     gpio;
};

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

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println();
  Serial.println("=== all-on: every PWM channel pinned to full duty ===");
  for (size_t i = 0; i < kChannelCount; ++i) {
    ledcSetup(i, kPwmFrequencyHz, kPwmResolutionBits);
    ledcAttachPin(kChannels[i].gpio, i);
    ledcWrite(i, kDutyFull);
    Serial.printf("  GPIO %2u  %s  -> HIGH\n",
                  kChannels[i].gpio, kChannels[i].label);
  }
  Serial.println("All channels driven. Loop is idle; probe at will.");
}

void loop() {
  // Nothing to do — outputs are latched by the LEDC peripheral.
  delay(1000);
}
