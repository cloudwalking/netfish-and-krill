#include "FastLED.h"

#define DATA_PIN 3
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS 50
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 90

// NUM_PIXELS where PIXELS refers the virtual LED string. 
// The virtual string is mapped to the physical string one or more times.
#define NUM_PIXELS 16
#define NUM_VIRTUAL_STRANDS 3

// For "baseColor" animation.
int8_t _basePointer[NUM_VIRTUAL_STRANDS] = { -1 };
CRGB _baseColor[NUM_VIRTUAL_STRANDS][NUM_PIXELS] = { 0 };

// Virtual LEDs.
CRGB _pixelBuffer[NUM_VIRTUAL_STRANDS][NUM_PIXELS] = { 0 };

// Physical LEDs.
CRGB _leds[NUM_LEDS] = { 0 };

DEFINE_GRADIENT_PALETTE(_whaleColors1) {
  0, 0, 38, 133,
  127, 0, 200, 224,
  255, 0, 149, 95
};

DEFINE_GRADIENT_PALETTE(_whaleColors2) {
  0, 0, 96, 196,
  255, 0, 196, 96
};

void setup() {
  delay(1000);
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
  fill_solid(_leds, NUM_LEDS, CRGB::Black);
}

void loop() {
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  unsigned long now = millis();

  crawlBaseColor(3000, now, &_basePointer[0], _baseColor[0], _pixelBuffer[0]);
  crawlBaseColor(5000, now + 2000, &_basePointer[1], _baseColor[1], _pixelBuffer[1]);
  crawlBaseColor(2500, now + 800, &_basePointer[2], _baseColor[2], _pixelBuffer[2]);

  render();
}

void crawlBaseColor(float durationMS, unsigned long nowMS, int8_t *progressIndex, CRGB *scratchBuffer, CRGB *outBuffer) {
  double fractionComplete = calculateFractionComplete(nowMS, durationMS);

  // Use integer to automatically round down.
  int8_t next = fractionComplete * NUM_PIXELS;
  // Subtract the whole value from the whole + frational value 
  // to get "% done with this pixel, until we switch to next pixel"
  float nextFraction = fractionComplete * NUM_PIXELS - next;

  // Fade the other pixels
  for (int8_t i = 0; i < NUM_PIXELS; i++ ) {
    if (i == next) { continue; }  
    scratchBuffer[i].fadeToBlackBy(1);
  }

  if (next != *progressIndex) {
    *progressIndex = next;
    scratchBuffer[next] =
      ColorFromPalette((CRGBPalette16)_whaleColors2, random8(), 255, LINEARBLEND);
    scratchBuffer[next].maximizeBrightness(255);
  }

  // Step brightness to max, then down to the % we actually want.
  scratchBuffer[next].maximizeBrightness(255);
  scratchBuffer[next].maximizeBrightness(max(5, nextFraction * 255));

  // For this animation we just wholesale replace the out buffer.
  for (int8_t i = 0; i < NUM_PIXELS; i++) {
    outBuffer[i] = scratchBuffer[i];
  }

//  Serial.print(next);
//  Serial.print("\t");
//  Serial.println(shouldFadeIn ? "yes" : "no");
}

float calculateFractionComplete(unsigned long nowMS, float durationMS) {
  return (nowMS % (unsigned long)durationMS) / durationMS;
}

// Render out the pixel buffer to the physical LEDs. This emulates 3 strands using one single LED strand.
void render() {
  for (int8_t i = 0; i < NUM_PIXELS; i++) {
    // 0 - 11
    _leds[i] = _pixelBuffer[0][i];
    // 23 - 12 (Backwards because we're one strand of LEDs and in this portion it's wrapped the other way.)
    _leds[2 * NUM_PIXELS - 1 - i] = _pixelBuffer[1][i];
    // 24 - 36
    _leds[i + 2 * NUM_PIXELS] = _pixelBuffer[2][i];
  }
}

