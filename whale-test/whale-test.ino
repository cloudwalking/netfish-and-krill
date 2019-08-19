#include "FastLED.h"

#define LED_TYPE WS2811
#define COLOR_ORDER RBG
#define BRIGHTNESS 255
#define FRAMES_PER_SECOND 72

#define NUM_PIXELS 70
#define NUM_STRIPS 8

// For "crawlBaseColor" animation.
int8_t _baseColorPointer[NUM_STRIPS] = { -1 };
CRGB _baseColor[NUM_STRIPS][NUM_PIXELS] = { 0 };

// Virtual LEDs.
CRGB _pixelBuffer[NUM_STRIPS][NUM_PIXELS] = { 0 };
// Physical LEDs.
CRGB _leds[NUM_STRIPS * NUM_PIXELS] = { 0 };

const int8_t _safetyLights[] = {
  7,
  20, 21,
  36, 37,
  58, 59,
  68
};

// Palettes at end of file.
//extern const TProgmemRGBGradientPalettePtr _palettes[];
extern const CRGBPalette16 _palettes[];
extern const uint8_t _numPalettes;
CRGBPalette16 _currentPalette;
CRGBPalette16 _targetPalette;
int8_t _palettePointer = 0;

// Palettes are [gradient fraction, red, green, blue] order.

DEFINE_GRADIENT_PALETTE(_whaleColors) {
  0, 0, 0, 255,
  255, 0, 255, 240
};

DEFINE_GRADIENT_PALETTE(_palette_back_blue) {
  0, 0, 64, 255,
  64, 32, 64, 255,
  128, 0, 16, 128,
  129, 0, 0, 64,
  255, 64, 0, 128
};

DEFINE_GRADIENT_PALETTE(_palette_back_turquoise) {
  0, 0, 192, 255,
  255, 0, 64, 64
};

DEFINE_GRADIENT_PALETTE(_palette_back_teal) {
  0, 0, 128, 255,
  255, 0, 64, 96
};

DEFINE_GRADIENT_PALETTE(_bellyColors) {
  // Desaturated blue.
  0, 140, 140, 255,
  // Bright teal.
  255, 0, 255, 255
};

DEFINE_GRADIENT_PALETTE(_back_blue) {
  0, 0, 64, 255,
  255, 0, 16, 128
};

DEFINE_GRADIENT_PALETTE(_belly_white_blue) {
  0, 201, 206, 255,
  255, 255, 255, 255
};

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  delay(1000);
  Serial.begin(9600);

  FastLED.addLeds<WS2811_PORTD, NUM_STRIPS, COLOR_ORDER>(_leds, NUM_PIXELS);
  FastLED.setBrightness(BRIGHTNESS);

  fill_solid(_leds, NUM_STRIPS * NUM_PIXELS, CRGB::Black);
  digitalWrite(LED_BUILTIN, HIGH);

  _currentPalette = _palettes[0];
  _targetPalette = _palettes[0];
}

void loop() {
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);

  updateColorPalette();
  crawlSameSpeeds();
  safetyLights();

  // Last, to render out.
  render_multi_strip();
}

///////////////////////////////////////

void updateColorPalette() {
  EVERY_N_SECONDS(10) {
    _palettePointer = (_palettePointer + 1) % _numPalettes;
    _targetPalette = _palettes[_palettePointer];
    Serial.print("_numPalettes: ");
    Serial.print(_numPalettes);
    Serial.print(" _palettePointer: ");
    Serial.println(_palettePointer);
  }

  EVERY_N_MILLISECONDS(40) {
    nblendPaletteTowardPalette(_currentPalette, _targetPalette, 16);
  }
}

//CRGB _safetyColors[];
void safetyLights() {
//  for (int8_t i = 0; i < sizeof(_safetyLights) / sizeof(int8_t); i++) {
////    _pixelBuffer[0][_safetyLights[i]] = ColorFromPalette(_currentPalette, random8(), 255, LINEARBLEND);
//    _pixelBuffer[0][_safetyLights[i]].maximizeBrightness(255);
//  }
}

void crawlSameSpeeds() {
  const unsigned long now = millis();
  const float baseDuration = 4999;

  const CRGBPalette16 back = _currentPalette;
  const CRGBPalette16 belly = _belly_white_blue;
  
  crawlBaseColor(baseDuration, now, &_baseColorPointer[0], _baseColor[0], _pixelBuffer[0], back);
  crawlBaseColor(baseDuration, now, &_baseColorPointer[1], _baseColor[1], _pixelBuffer[1], back);
  crawlBaseColor(baseDuration, now, &_baseColorPointer[2], _baseColor[2], _pixelBuffer[2], back);
  crawlBaseColor(baseDuration, now, &_baseColorPointer[3], _baseColor[3], _pixelBuffer[3], back);
  crawlBaseColor(baseDuration, now, &_baseColorPointer[4], _baseColor[4], _pixelBuffer[4], belly);
  crawlBaseColor(baseDuration, now, &_baseColorPointer[5], _baseColor[5], _pixelBuffer[5], belly);
}

void crawlDifferentSpeeds() {
  const unsigned long now = millis();
  const float baseDuration = 5000;

  // Back
  crawlBaseColor(1.0 * baseDuration, now + 1200, &_baseColorPointer[5], _baseColor[5], _pixelBuffer[5], _palette_back_turquoise);
  crawlBaseColor(0.95 * baseDuration, now + 1000, &_baseColorPointer[4], _baseColor[4], _pixelBuffer[4], _palette_back_teal);
  crawlBaseColor(baseDuration, now, &_baseColorPointer[0], _baseColor[0], _pixelBuffer[0], _palette_back_blue);

  crawlBaseColor(baseDuration * 0.8, now, &_baseColorPointer[6], _baseColor[6], _pixelBuffer[6], _palette_back_blue);
  crawlBaseColor(baseDuration * 0.7, now, &_baseColorPointer[7], _baseColor[7], _pixelBuffer[7], _palette_back_blue);

  // Belly
  crawlBaseColor(0.9 * baseDuration, now + 2500, &_baseColorPointer[1], _baseColor[1], _pixelBuffer[1], _bellyColors);
  crawlBaseColor(1.1 * baseDuration, now + 500, &_baseColorPointer[2], _baseColor[2], _pixelBuffer[2], _bellyColors);
  crawlBaseColor(1.05 * baseDuration, now + 800, &_baseColorPointer[3], _baseColor[3], _pixelBuffer[3], _bellyColors);

  render_multi_strip();
}

///////////////////////////////////////

void crawlBaseColor(float durationMS, unsigned long nowMS, int8_t *progressIndex, CRGB *scratchBuffer, CRGB *outBuffer, CRGBPalette16 palette) {
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
  // Make fade out take a bit longer when duration is longer.
  int8_t fadeOutAdjust = durationMS >= 3000 ? -1 * durationMS / 1200 : 0;
  for (int8_t i = 0; i < NUM_PIXELS; i++ ) {
    if (i == target) {
      continue;
    }
    scratchBuffer[i].fadeToBlackBy(max(1, fadeOutFraction + fadeOutAdjust));
  }

  // TODO: Fade using outBuffer instead of scratch. Track colors in scratchBuffer and fade target outBuffer to scratchBuffer color?

  if (target != *progressIndex) {
    *progressIndex = target;
    scratchBuffer[target] =
      ColorFromPalette((CRGBPalette16)palette, random8(), 255, LINEARBLEND);
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

//    Serial.print(durationMS);
//    Serial.print("\t");
//    Serial.println(fractionComplete);
}

float calculateFractionComplete(unsigned long nowMS, float durationMS) {
  return (nowMS % (unsigned long)durationMS) / durationMS;
}

// For rendering strangely shaped things.
//void render_multi_strip_virtual() {
//
//  // send odd strips forward, even strips backward
//
//  for (int8_t strip = 0; strip < NUM_STRIPS; strip++) {
//    for (int8_t pixel = 0; pixel < NUM_PIXELS; pixel++) {
//      int16_t index = 0;
//
//      // Janky hack for current demo. Even strips play in reverse.
//      bool isReversed = strip % 2 == 0;
//
//      if (isReversed) {
//        index = strip * NUM_PIXELS + NUM_PIXELS - pixel;
//      } else {
//        index = strip * NUM_PIXELS + pixel;
//      }
//
//      _leds[index] = _pixelBuffer[strip][pixel];
////      Serial.print(index);
////      Serial.print(": ");
////      Serial.print(_pixelBuffer[strip][pixel]);
//    }
////    Serial.println();
//  }
//}

// Render out the pixel buffer to the physical LEDs. Uses multiple LED strands.
void render_multi_strip() {
  bool shouldLog = false;
  if (shouldLog) { Serial.println(">>>>>>>>>"); }
  
  for (int8_t strip = 0; strip < NUM_STRIPS; strip++) {
    for (int8_t pixel = 0; pixel < NUM_PIXELS; pixel++) {
      int16_t index = strip * NUM_PIXELS + pixel;
      _leds[index] = _pixelBuffer[strip][pixel];
      
      if (shouldLog) {
        // Prints out each pixel (r,g,b) for each strip.
        Serial.print("i");
        Serial.print(index);
        Serial.print(":s");
        Serial.print(strip);
        Serial.print("p");
        Serial.print(pixel);
        Serial.print("=(");
        Serial.print(_pixelBuffer[strip][pixel].red);
        Serial.print(",");
        Serial.print(_pixelBuffer[strip][pixel].green);
        Serial.print(",");
        Serial.print(_pixelBuffer[strip][pixel].blue);
        Serial.print(") ");
      }
    }
    if (shouldLog) { Serial.println(); }
  }
  if (shouldLog) { Serial.println("<<<<<<<<<"); }
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

////////////////////////////////////////////

// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/BlacK_Red_Magenta_Yellow.png.index.html
DEFINE_GRADIENT_PALETTE(Red_Magenta_Yellow_gp) {
   84, 255,  0,  0,
  127, 255,  0, 45,
  170, 255,  0,255,
  212, 255, 55, 45,
  255, 255,255,  0};

// http://soliton.vm.bytemark.co.uk/pub/cpt-city/nd/basic/tn/Blue_Cyan_Yellow.png.index.html
DEFINE_GRADIENT_PALETTE(Blue_Cyan_Yellow_gp) {
    0,   0,  0,255,
   63,   0, 55,255,
  127,   0,255,255,
  191,  42,255, 45,
  255, 255,255,  0};

const CRGBPalette16 _palettes[] = {
  _back_blue,
  RainbowColors_p,
  OceanColors_p,
  ForestColors_p,
  PartyColors_p,
  HeatColors_p,
  Blue_Cyan_Yellow_gp,
  Red_Magenta_Yellow_gp
};
 
const uint8_t _numPalettes = sizeof(_palettes) / sizeof(CRGBPalette16);
 
