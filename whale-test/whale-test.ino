#include "FastLED.h"

#define LED_TYPE WS2811
#define COLOR_ORDER RBG
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 72

#define NUM_PIXELS 18
#define NUM_STRIPS 3

// For "crawlBaseColor" animation.
int8_t _baseColorPointer[NUM_STRIPS] = { -1 };
CRGB _baseColor[NUM_STRIPS][NUM_PIXELS] = { 0 };

// Virtual LEDs.
CRGB _pixelBuffer[NUM_STRIPS][NUM_PIXELS] = { 0 };
// Physical LEDs.
CRGB _leds[NUM_STRIPS * NUM_PIXELS] = { 0 };

DEFINE_GRADIENT_PALETTE(_whaleColors) {
  0, 0, 0, 255,
  255, 0, 255, 240
};

void setup() {
  delay(1000);
  Serial.begin(9600);
  
  FastLED.addLeds<WS2811_PORTD, NUM_STRIPS, COLOR_ORDER>(_leds, NUM_PIXELS);
  FastLED.setBrightness(BRIGHTNESS);
  
  fill_solid(_leds, NUM_STRIPS * NUM_PIXELS, CRGB::Black);
}

void loop() {
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  unsigned long now = millis();

  float baseDuration = 4000;

  crawlBaseColor(baseDuration, now, &_baseColorPointer[0], _baseColor[0], _pixelBuffer[0]);
//  crawlBaseColor(1.6 * baseDuration, now + 2000, &_baseColorPointer[1], _baseColor[1], _pixelBuffer[1]);
//  crawlBaseColor(0.8 * baseDuration, now + 800, &_baseColorPointer[2], _baseColor[2], _pixelBuffer[2]);
  crawlBaseColor(1.0 * baseDuration, now + 2000, &_baseColorPointer[1], _baseColor[1], _pixelBuffer[1]);
  crawlBaseColor(1.0 * baseDuration, now + 800, &_baseColorPointer[2], _baseColor[2], _pixelBuffer[2]);

  render_multi_strip();
}

void crawlBaseColor(float durationMS, unsigned long nowMS, int8_t *progressIndex, CRGB *scratchBuffer, CRGB *outBuffer) {
  const bool fadeColorIn = true;
  
  double fractionComplete = calculateFractionComplete(nowMS, durationMS);

  // Use integer to automatically round down.
  int8_t target = fractionComplete * NUM_PIXELS;
  // Subtract the whole value from the whole + frational value 
  // to get "% done with this pixel, until we switch to target pixel"
  float targetFraction = fractionComplete * NUM_PIXELS - target;

  // Fade the other pixels out.
  // How many steps (as %) does it take to fade out, based on our framerate and speed?
  int8_t fadeOutFraction = 100 / (durationMS / FRAMES_PER_SECOND);
  int8_t fadeOutAdjust = durationMS >= 3000 ? -2 : 0;
  for (int8_t i = 0; i < NUM_PIXELS; i++ ) {
    if (i == target) { continue; }  
    scratchBuffer[i].fadeToBlackBy(max(1, fadeOutFraction + fadeOutAdjust));
  }

  // TODO: Fade using outBuffer instead of scratch. Track colors in scratchBuffer and fade target outBuffer to scratchBuffer color?

  if (target != *progressIndex) {
    *progressIndex = target;
    scratchBuffer[target] =
      ColorFromPalette((CRGBPalette16)_whaleColors, random8(), 255, LINEARBLEND);
    scratchBuffer[target].maximizeBrightness(255);
  }

  if (fadeColorIn) {
    // Step brightness to max, then down to the % we actually want.
    scratchBuffer[target].maximizeBrightness(255);
    const int8_t minBrightness = 5;
    scratchBuffer[target].maximizeBrightness(max(minBrightness, targetFraction * 255));
  }

  // For this animation we just wholesale replace the out buffer.
  for (int8_t i = 0; i < NUM_PIXELS; i++) {
    outBuffer[i] = scratchBuffer[i];
  }

//  Serial.print(durationMS);
//  Serial.print("\t");
//  Serial.println(fractionComplete);
}

float calculateFractionComplete(unsigned long nowMS, float durationMS) {
  return (nowMS % (unsigned long)durationMS) / durationMS;
}

// Render out the pixel buffer to the physical LEDs. Uses multiple LED strands.
void render_multi_strip() {
  for (int8_t strip = 0; strip < NUM_STRIPS; strip++) {
    for (int8_t pixel = 0; pixel < NUM_PIXELS; pixel++) {
      int8_t index = strip * NUM_PIXELS + pixel;
      _leds[index] = _pixelBuffer[strip][pixel];
//      Serial.print(index);
//      Serial.print(": ");
//      Serial.print(_pixelBuffer[strip][pixel]);
    }
//    Serial.println();
  }
}

// Not currently used.
//// Render out the pixel buffer to the physical LEDs. This emulates 3 strands using one single LED strand.
//void render_virtual() {
//  /*
//    data -> ----------    _pixelBuffer[0]
//                     |
//            ----------    _pixelBuffer[1]
//            |
//            ----------    _pixelBuffer[2]
//  */
//  for (int8_t i = 0; i < NUM_PIXELS; i++) {
//    // 0 - 11
//    _leds[i] = _pixelBuffer[0][i];
//    // 23 - 12 (Backwards -- one strand of LEDs, this is the middle section.)
//    _leds[2 * NUM_PIXELS - 1 - i] = _pixelBuffer[1][i];
//    // 24 - 36
//    _leds[i + 2 * NUM_PIXELS] = _pixelBuffer[2][i];
//  }
//}
