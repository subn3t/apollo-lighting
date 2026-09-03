// HT16K33 4-digit 7-segment backpack — power-on demo.
//
// A tasteful aerospace-instrument sequence for the CCT / lumens readouts:
// a lamp test, a perimeter "scanning" sweep, a colour-temperature ramp that
// settles on the project's 4100K default, and a lumens count that exercises
// both display formats (plain integer, then the compressed D.DDD form).
// Throwaway test sketch — not part of the firmware build.
#include <Arduino.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"

constexpr int     kSdaPin         = 21;
constexpr int     kSclPin         = 22;
constexpr uint8_t kDisplayAddress = 0x70;   // default backpack address
constexpr uint8_t kBrightnessFull = 15;     // HT16K33 levels: 0..15

Adafruit_7segment display = Adafruit_7segment();

// Digit slots on the HT16K33 7-seg map (slot 2 is the colon, so it's skipped).
constexpr uint8_t kDigit[4] = {0, 1, 3, 4};

// Segment bits for writeDigitRaw(): bit0=A(top) .. bit5=F(top-left),
// bit6=G(middle), bit7=decimal point.
constexpr uint8_t kSegA = 1 << 0, kSegB = 1 << 1, kSegC = 1 << 2;
constexpr uint8_t kSegD = 1 << 3, kSegE = 1 << 4, kSegF = 1 << 5;

// One lit segment on the outer border. Walking this list chases a streak
// around the whole 4-digit display: across the top, down the right edge,
// along the bottom, up the left edge.
struct BorderSeg { uint8_t slot; uint8_t seg; };
const BorderSeg kBorder[] = {
  {0, kSegA}, {1, kSegA}, {3, kSegA}, {4, kSegA},   // top edge, left to right
  {4, kSegB}, {4, kSegC},                           // right edge, downward
  {4, kSegD}, {3, kSegD}, {1, kSegD}, {0, kSegD},   // bottom edge, right to left
  {0, kSegE}, {0, kSegF},                           // left edge, upward
};
constexpr int kBorderLen = sizeof(kBorder) / sizeof(kBorder[0]);

void blankDisplay() {
  display.clear();
  display.drawColon(false);
  display.writeDisplay();
}

// Every segment lit for a beat — the avionics "press to test" check.
void lampTest() {
  for (uint8_t d = 0; d < 4; d++) display.writeDigitRaw(kDigit[d], 0xFF);
  display.drawColon(true);
  display.writeDisplay();
  delay(900);
  blankDisplay();
}

// Chase a short streak around the display border.
void borderSweep(int laps) {
  constexpr int kTrail = 3;
  for (int lap = 0; lap < laps; lap++) {
    for (int head = 0; head < kBorderLen; head++) {
      uint8_t buf[5] = {0, 0, 0, 0, 0};
      for (int t = 0; t < kTrail; t++) {
        const BorderSeg& s = kBorder[(head - t + kBorderLen) % kBorderLen];
        buf[s.slot] |= s.seg;
      }
      for (uint8_t d = 0; d < 4; d++) display.writeDigitRaw(kDigit[d], buf[kDigit[d]]);
      display.writeDisplay();
      delay(45);
    }
  }
  blankDisplay();
}

// Ramp the colour temperature from the strip's warm endpoint (3200K) up to 6500K,
// back down, and settle on 4100K.
void cctSweep() {
  for (int k = 3200; k <= 6500; k += 40) { display.print(k); display.writeDisplay(); delay(11); }
  delay(250);
  for (int k = 6500; k >= 4100; k -= 40) { display.print(k); display.writeDisplay(); delay(11); }
  display.print(4100);
  display.writeDisplay();
  delay(700);
}

// Show a lumens value the way the real readout will: a plain integer below
// 10,000 lm, or a compressed "D.DDD" (lit decimal point = x10,000) above it.
void showLumens(long total) {
  display.clear();
  display.drawColon(false);
  if (total < 10000) {
    display.print(total);
  } else {
    long v = total / 10;                               // 10000..20000 -> 1000..2000
    display.writeDigitNum(0, (v / 1000) % 10, true);   // decimal point lit
    display.writeDigitNum(1, (v / 100) % 10);
    display.writeDigitNum(3, (v / 10) % 10);
    display.writeDigitNum(4, v % 10);
  }
  display.writeDisplay();
}

void lumensRamp() {
  for (long lm = 0; lm <= 9999; lm += 117) { showLumens(lm); delay(9); }
  showLumens(9999);
  delay(250);
  for (long lm = 10000; lm <= 20000; lm += 131) { showLumens(lm); delay(11); }
  showLumens(20000);
  delay(600);
}

// Fade the whole display in and out a few times — the DSKY "glow".
void breathe(int cycles) {
  display.print(4100);
  display.writeDisplay();
  for (int c = 0; c < cycles; c++) {
    for (int b = 1; b <= 15; b++) { display.setBrightness(b); delay(38); }
    for (int b = 15; b >= 1; b--) { display.setBrightness(b); delay(38); }
  }
  display.setBrightness(kBrightnessFull);
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Wire.begin(kSdaPin, kSclPin);
  if (!display.begin(kDisplayAddress)) {
    Serial.printf("No HT16K33 at 0x%02X - check wiring.\n", kDisplayAddress);
  } else {
    Serial.println("HT16K33 found - running demo.");
  }
  display.setBrightness(kBrightnessFull);
  blankDisplay();
}

void loop() {
  Serial.println("lamp test");    lampTest();
  Serial.println("border sweep"); borderSweep(3);
  Serial.println("CCT sweep");    cctSweep();
  Serial.println("lumens ramp");  lumensRamp();
  Serial.println("breathe");      breathe(2);
  blankDisplay();
  delay(500);
}
