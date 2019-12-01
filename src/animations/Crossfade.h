#include <MyKnob.h>
#include "Animation.h"
#include "FastLED.h"

class Crossfade : public Animation {
 private:
  int initialPosition = 40;
  int start = 5;
  int finish = 200;
  bool initialized = false;
  bool loopRotary = false;
  MyKnob &knob;
  CRGB *leds;

 public:
  Crossfade(MyKnob &knob_, CRGB leds_[]) : knob(knob_), leds(leds_) {}

  void setup() {
    knob.setDefaults(initialPosition, start, finish, loopRotary);
  };

  void loop() {
    int dlay = knob.confine();
    static uint8_t hue = 0;
    if (nonBlockDelay(dlay)) {
      // FastLED.showColor(CHSV(hue++, 255, 255));
      fill_solid(leds, NUMPIXELS, CHSV(hue++, 255, 255));
    }
  }

};
