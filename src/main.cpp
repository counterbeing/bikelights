#include <Arduino.h>
#define FASTLED_INTERNAL
#include <Encoder.h>
#include <MyKnob.h>
#include <SPI.h>
#include "FastLED.h"

#include "SwarmMap.h"

#include "Radio.h"
#include "config.h"

#include "animations/ColorChooser.h"
#include "animations/Crossfade.h"
#include "animations/DiamondNecklace.h"
#include "animations/Dimmer.h"
#include "animations/FuckMyEyes.h"
#include "animations/LightningButt.h"
#include "animations/Race.h"
#include "animations/Rainbow.h"
#include "animations/Stars.h"
#include "animations/Stripes.h"

// Pins for the rotariy
uint8_t rotary1 = 2;
uint8_t rotary2 = 3;

int buttonPin = A0;
bool offMode = false;

CRGB leds[NUMPIXELS];
int feedbackPattern = -1;
MyKnob knob(rotary1, rotary2, offMode, feedbackPattern);

// Load animations...
Crossfade crossfade(knob, leds);
ColorChooser color_chooser(knob, leds);
Race race(knob, leds);
Stars stars(knob, leds);
Rainbow rainbow(knob, leds);
FuckMyEyes fuck_my_eyes(knob, leds);
LightningButt lightning_butt(knob, leds);
Stripes stripes(knob, leds);
DiamondNecklace diamond_necklace(knob, leds);
Dimmer dimmer(knob, leds);

SwarmMap swarmMap;

int animation_index = 0;
Radio radio(knob, animation_index);


// Animation *current_animation = &rainbow;
Animation *current_animation = &crossfade;
int previous_animation_index = -1;

int feedbackLength = 800;
long feedbackEnd = (millis() + feedbackLength);

void runAdjustments() {
  if (feedbackPattern < 0) {
    return;
  }
  if (feedbackPattern > 0) {
    // Serial.println("STARTING FEEBACK");
    feedbackEnd = (millis() + feedbackLength);
  }
  feedbackPattern = 0;

  int currentLapse = feedbackEnd - millis();
  if (currentLapse < 0) {
    feedbackPattern = -1;
  }

  CRGB color1;
  CRGB color2;

  switch (feedbackPattern) {
    case 0:
      color1 = CRGB::Blue;
      color2 = CRGB::Green;
      break;
    case 1:
      color1 = CRGB::Black;
      color2 = CRGB::Red;
      break;
  }

  if ((currentLapse / 100) % 2 == 0) {
    fill_solid(leds, NUMPIXELS, color1);
  } else {
    fill_solid(leds, NUMPIXELS, color2);
  }
}
void playAnimation() {
  if (animation_index != previous_animation_index) {
    if (animation_index > 8) animation_index = 0;
    // BUG CAUTION
    // never follow one animation function immediately with itself in the the
    // next case

    if (KNOB_DEBUG) {
      Serial.print("Animation Index: ");
      Serial.println(animation_index);
    }

    switch (animation_index) {
      case 0:
        current_animation = &lightning_butt;
        break;
      case 1:
        current_animation = &crossfade;
        break;
      case 2:
        current_animation = &color_chooser;
        break;
      case 3:
        current_animation = &race;
        break;
      case 4:
        current_animation = &stars;
        break;
      case 5:
        current_animation = &rainbow;
        break;
      case 6:
        current_animation = &fuck_my_eyes;
        break;
      case 7:
        current_animation = &stripes;
        break;
      case 8:
        current_animation = &diamond_necklace;
        break;
      case 9:
        current_animation = &dimmer;
        break;
      default:
        // Serial.println("\n\nWARN: default animation switch case");
        break;
    }
    current_animation->setup();
    previous_animation_index = animation_index;
  }
  current_animation->run();

  runAdjustments();

  FastLED.show();
}

void setup() {
#ifdef SCARF_WS2811
  FastLED.addLeds<WS2811, DATAPIN, GRB>(leds, NUMPIXELS);
#endif
#ifdef SCARF_SK9822
  FastLED.addLeds<SK9822, DATAPIN, CLOCKPIN, BGR>(leds, NUMPIXELS);
#endif

#ifdef SCARF_DOTSTAR
  FastLED.addLeds<DOTSTAR, DATAPIN, CLOCKPIN, BGR>(leds, NUMPIXELS);
#endif

#ifdef BIGRED_WS2815
  FastLED.addLeds<WS2811, DATAPIN, BGR>(leds, NUMPIXELS);
#endif
  randomSeed(analogRead(0));

  Serial.begin(57600);

  button_debouncer.attach(buttonPin, INPUT_PULLUP);
  button_debouncer.interval(5);
  radio.setup();
}

void loop() {
  button_debouncer.update();
  knob.check(&animation_index);
  if (offMode) {
    fill_solid(leds, NUMPIXELS, CRGB::Black);
    FastLED.show();
    return;
  }
  // printf("swarmSize: %d", swarmMap.getSwarmSize()); 
  swarmMap.getSwarmSize();
  playAnimation();
  radio.check();
}