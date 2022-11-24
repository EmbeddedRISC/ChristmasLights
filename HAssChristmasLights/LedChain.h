#include "FastLED.h"

class SmartLED {

  public:
    CRGB *led;

    void set(CRGB val) {
      *(this->led) = val;
      this->led_target = val;
      this->transition_steps = 0;
    }

    void transition(CRGB val, unsigned short steps) {
      this->led_target = val;
      this->transition_steps = steps;
    }

    void update() {
      if (*(this->led) == this->led_target) {
        return;
      }

      // CRGB delta = this->led_target - *(this->led);
      // Serial.println(delta.r);
      // delta /= this->transition_steps;
      // Serial.println(delta.r);
      // Serial.println();

      short r_delta = ((short)this->led_target.r - (short)this->led->r) / this->transition_steps;
      short g_delta = ((short)this->led_target.g - (short)this->led->g) / this->transition_steps;
      short b_delta = ((short)this->led_target.b - (short)this->led->b) / this->transition_steps;

      this->led->r += r_delta;
      this->led->g += g_delta;
      this->led->b += b_delta;
      this->transition_steps--;
      Serial.println(r_delta);
      Serial.println(this->transition_steps);
      Serial.println();
    }

    bool isTransitioning() {
      return this->transition_steps > 0;
    }

  private:
    CRGB  led_target;
    short transition_steps;

};