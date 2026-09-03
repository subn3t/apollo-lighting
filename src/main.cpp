#include <Arduino.h>

constexpr int kWarmBiasPotPin = 36;
constexpr int kCoolBiasPotPin = 39;
constexpr int kStrip1WarmPin  = 14;
constexpr int kStrip1CoolPin  = 12;

constexpr int      kWarmPwmChannel    = 0;
constexpr int      kCoolPwmChannel    = 1;
constexpr uint32_t kPwmFrequencyHz    = 5000;
constexpr uint8_t  kPwmResolutionBits = 12;

void setup() {
  Serial.begin(115200);
  delay(100);
  Serial.println("Apollo slice-2: WARM/COOL BIAS pots drive Strip 1 WARM/COOL PWM");

  analogReadResolution(12);
  ledcSetup(kWarmPwmChannel, kPwmFrequencyHz, kPwmResolutionBits);
  ledcAttachPin(kStrip1WarmPin, kWarmPwmChannel);
  ledcSetup(kCoolPwmChannel, kPwmFrequencyHz, kPwmResolutionBits);
  ledcAttachPin(kStrip1CoolPin, kCoolPwmChannel);
}

void loop() {
  int warm = analogRead(kWarmBiasPotPin);
  int cool = analogRead(kCoolBiasPotPin);
  ledcWrite(kWarmPwmChannel, warm);
  ledcWrite(kCoolPwmChannel, cool);

  static uint32_t lastPrint = 0;
  uint32_t now = millis();
  if (now - lastPrint >= 200) {
    lastPrint = now;
    Serial.printf("WARM=%4d  COOL=%4d\n", warm, cool);
  }
}
