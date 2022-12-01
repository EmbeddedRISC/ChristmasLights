/*************************************************************************
 * Arduino Code for WS2811 HAss XMas Lights
 * Copyright (C) <year>  <name of author>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 ***************************************************************************/


#include "SmartLED.h"

#define GARAGE_LED_PIN  D0
#define GARAGE_LED_SIZE 150

#define DOORWAY_LED_PIN  D1
#define DOORWAY_LED_SIZE 100

#define ROOF_LED_PIN  D2
#define ROOF_LED_SIZE 150

CRGB garage_raw_leds[GARAGE_LED_SIZE];
SmartLED garage_leds[GARAGE_LED_SIZE];

CRGB doorway_raw_leds[DOORWAY_LED_SIZE];
SmartLED doorway_leds[DOORWAY_LED_SIZE];

CRGB roof_raw_leds[ROOF_LED_SIZE];
SmartLED roof_leds[ROOF_LED_SIZE];

void setup() {
  Serial.begin(500000);

  FastLED.addLeds<WS2811, GARAGE_LED_PIN>(garage_raw_leds, GARAGE_LED_SIZE);

  init_chain(garage_leds, garage_raw_leds, GARAGE_LED_SIZE, CRGB::Black);
  twinkle_init(garage_leds, GARAGE_LED_SIZE, CRGB::Green, CRGB::Red);

  init_chain(doorway_leds, doorway_raw_leds, DOORWAY_LED_SIZE, CRGB::Black);
  twinkle_init(doorway_leds, DOORWAY_LED_SIZE, CRGB::Green, CRGB::Red);

  init_chain(roof_leds, roof_raw_leds, ROOF_LED_SIZE, CRGB::Black);
  twinkle_init(roof_leds, ROOF_LED_SIZE, CRGB::Green, CRGB::Red);

  FastLED.show();

  // Use Random noise on analog pin for randomness
  randomSeed(analogRead(A0));
}

void loop() {
  twinkle_loop(garage_leds, GARAGE_LED_SIZE, CRGB::Green, CRGB::Red);
  twinkle_loop(doorway_leds, DOORWAY_LED_SIZE, CRGB::Green, CRGB::Red);
  twinkle_loop(roof_leds, ROOF_LED_SIZE, CRGB::Green, CRGB::Red);

  FastLED.show();

  delay(50);
}
