#include "FastLED.h"

#define DATA_PIN 3
#define LED_TYPE WS2811
#define COLOR_ORDER RGB
#define NUM_LEDS 50
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 72

// NUM_PIXELS where PIXELS refers the virtual LED string. 
// The virtual string is mapped to the physical string one or more times.
#define NUM_PIXELS 16
#define NUM_VIRTUAL_STRANDS 3

// For "crawlBaseColor" animation.
int8_t _basePointer[NUM_VIRTUAL_STRANDS] = { -1 };
CRGB _baseColor[NUM_VIRTUAL_STRANDS][NUM_PIXELS] = { 0 };

// Virtual LEDs.
CRGB _pixelBuffer[NUM_VIRTUAL_STRANDS][NUM_PIXELS] = { 0 };
// Physical LEDs.
CRGB _leds[NUM_LEDS] = { 0 };

DEFINE_GRADIENT_PALETTE(_whaleColors) {
  0, 0, 0, 255,
  255, 0, 255, 240
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
  const bool fadeColorIn = true;
  
  double fractionComplete = calculateFractionComplete(nowMS, durationMS);

  // Use integer to automatically round down.
  int8_t next = fractionComplete * NUM_PIXELS;
  // Subtract the whole value from the whole + frational value 
  // to get "% done with this pixel, until we switch to next pixel"
  float nextFraction = fractionComplete * NUM_PIXELS - next;

  // Fade the other pixels out.
  // How many steps (as %) does it take to fade out, based on our framerate and speed?
  int8_t fadeOutFraction = 100 / (durationMS / FRAMES_PER_SECOND);
  for (int8_t i = 0; i < NUM_PIXELS; i++ ) {
    if (i == next) { continue; }  
    scratchBuffer[i].fadeToBlackBy(max(1, fadeOutFraction));
  }

  if (next != *progressIndex) {
    *progressIndex = next;
    scratchBuffer[next] =
      ColorFromPalette((CRGBPalette16)_whaleColors, random8(), 255, LINEARBLEND);
    scratchBuffer[next].maximizeBrightness(255);
  }

  if (fadeColorIn) {
    // Step brightness to max, then down to the % we actually want.
    scratchBuffer[next].maximizeBrightness(255);
    scratchBuffer[next].maximizeBrightness(max(5, nextFraction * 255));
  }

  // For this animation we just wholesale replace the out buffer.
  for (int8_t i = 0; i < NUM_PIXELS; i++) {
    outBuffer[i] = scratchBuffer[i];
  }

//  Serial.print(durationMS);
//  Serial.print("\t");
//  Serial.println(fadeOutActual);
}

float calculateFractionComplete(unsigned long nowMS, float durationMS) {
  return (nowMS % (unsigned long)durationMS) / durationMS;
}

// Render out the pixel buffer to the physical LEDs. This emulates 3 strands using one single LED strand.
void render() {
  /*
    data -> ----------    _pixelBuffer[0]
                     |
            ----------    _pixelBuffer[1]
            |
            ----------    _pixelBuffer[2]
  */
  for (int8_t i = 0; i < NUM_PIXELS; i++) {
    // 0 - 11
    _leds[i] = _pixelBuffer[0][i];
    // 23 - 12 (Backwards -- one strand of LEDs, this is the middle section.)
    _leds[2 * NUM_PIXELS - 1 - i] = _pixelBuffer[1][i];
    // 24 - 36
    _leds[i + 2 * NUM_PIXELS] = _pixelBuffer[2][i];
  }
}
