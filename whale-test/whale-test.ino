#include "FastLED.h"

#define DATA_PIN 3
#define LED_TYPE WS2811
#define COLOR_ORDER GRB
#define NUM_LEDS 50
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 120

#define NUM_PIXELS 12
CRGB _baseColor[NUM_PIXELS] = { 0 };
int _basePointer = -1;
CRGB _pixelBuffer[NUM_PIXELS] = { 0 };
CRGB _leds[NUM_LEDS] = { 0 };

DEFINE_GRADIENT_PALETTE(_whaleColors) {
//  0, 0, 48, 170,
//  255, 0, 174, 156
  0, 0, 38, 133,
  127, 0, 200, 224,
  255, 0, 149, 95
};

void setup() {
  delay(1000);
  Serial.begin(9600);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(_leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);
//  fill_solid(_leds, NUM_LEDS, CRGB::Black);
}

void loop() {
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  unsigned long now = millis();

  drawBaseColor(now);
  

  for (int i = 0; i < NUM_PIXELS; i++) {
    _pixelBuffer[i] = _baseColor[i];
  }

  render();
}

void drawBaseColor(unsigned long nowMS) {
  const float durationMS = 3000;
  double fractionComplete = calculateFractionComplete(nowMS, durationMS);

  // Use integer to automatically round down.
  int next = fractionComplete * NUM_PIXELS;
  // Subtract the whole value from the whole + frational value 
  // to get "% done with this pixel, until we switch to next pixel"
  float nextFraction = fractionComplete * NUM_PIXELS - next;

  // Fade the other pixels
  for (int i = 0; i < NUM_PIXELS; i++ ) {
    if (i == next) { continue; }  
    _baseColor[i].fadeToBlackBy(3);
  }

  if (next != _basePointer) {
    _basePointer = next;
    // Use 1 for brightness, we will fade it in.
    _baseColor[next] = ColorFromPalette((CRGBPalette16)_whaleColors, random8(), 1, LINEARBLEND);
  }

  // Step brightness to max, then down to the % we actually want.
  _baseColor[next].maximizeBrightness(255);
  _baseColor[next].maximizeBrightness(nextFraction * 255);

  Serial.print(nextFraction);
  Serial.print("\t");
  Serial.println(255 * nextFraction);
}

float calculateFractionComplete(unsigned long nowMS, float durationMS) {
  return (nowMS % (unsigned long)durationMS) / durationMS;
}

// Render out the pixel buffer to the physical LEDs. This emulates 3 strands using one single LED strand.
void render() {
  for (int i = 0; i < NUM_PIXELS; i++) {
    // 0 - 11
    _leds[i] = _pixelBuffer[i];
    // 23 - 12 (Backwards because we're one strand of LEDs and in this portion it's wrapped the other way.)
    _leds[2 * NUM_PIXELS - 1 - i] = _pixelBuffer[i];
    // 24 - 36
    _leds[i + 2 * NUM_PIXELS] = _pixelBuffer[i];
  }
}

