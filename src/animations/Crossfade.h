// #include <MyKnob.h>
#include "Animation.h"
// #include "FastLED.h"
#include "MilliTimer.h"

class Crossfade : public Animation {
 private:
  KnobSetting delay{35, 5, 200, false};
  MilliTimer timer{};
  uint8_t hue = 0;

 public:
  Crossfade(CRGB leds_[]) : Animation(leds_) {}

  void wakeUp() override {
    delay.activate();
  }

  void update() override {
    if (delay.update()) {
      configChangeFlag = true;
    }

    if (timer.hasElapsedWithReset(delay.get())) {
      fill_solid(leds, NUMPIXELS, CHSV(hue++, 255, 255));
    }
  }

};
