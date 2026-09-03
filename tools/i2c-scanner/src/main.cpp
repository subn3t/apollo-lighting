// I2C bus scanner — confirms an HT16K33 display backpack is wired correctly.
// Uses the same SDA/SCL pins as the real firmware so the test mirrors the build.
// Expect 0x70 for a default backpack, 0x71 once the A0 jumper is bridged.
#include <Arduino.h>
#include <Wire.h>

constexpr int kSdaPin = 21;
constexpr int kSclPin = 22;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("=== I2C scanner - HT16K33 backpack test ===");
  Serial.printf("SDA = GPIO%d   SCL = GPIO%d\n", kSdaPin, kSclPin);
  Wire.begin(kSdaPin, kSclPin);
}

void loop() {
  Serial.println("Scanning...");
  int found = 0;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      found++;
      Serial.printf("  device at 0x%02X", addr);
      if (addr == 0x70) Serial.print("   <- HT16K33 (default address)");
      else if (addr == 0x71) Serial.print("   <- HT16K33 (A0 jumper bridged)");
      Serial.println();
    }
  }
  if (found == 0) {
    Serial.println("  none found - check +/-/SDA/SCL wiring and reflow joints");
  }
  Serial.printf("Done (%d found). Rescanning in 3s...\n\n", found);
  delay(3000);
}
