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

#include <RH_RF95.h>
#include <RHReliableDatagram.h>

#include "SmartLED.h"
#include "XMasComms.h"

/////////////////////////////////////////////////////////////
// Debug Setup
/////////////////////////////////////////////////////////////

#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
  #define DEBUG_INIT() Serial.begin(500000);
  #define DEBUG_PRINT(x) Serial.print(x);
  #define DEBUG_PRINTLN(x) Serial.println(x);
#else
  #define DEBUG_INIT()
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

/////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////

#define LED_COUNT 200
#define LED_PIN A3

#define REFRESH_RATE_MS 50

/////////////////////////////////////////////////////////////
// LoRa Definition
/////////////////////////////////////////////////////////////

#define LORA_CSN_PIN 10
#define LORA_IRQ_PIN 2

RH_RF95 driver(LORA_CSN_PIN, LORA_IRQ_PIN);

/////////////////////////////////////////////////////////////
// LED Definition
/////////////////////////////////////////////////////////////

LedInfo led_info;

unsigned long lastLEDUpdate;
LedChain<LED_COUNT, LED_PIN> leds(DEFAULT_TRANSITION_TIME, REFRESH_RATE_MS, DEFAULT_EFFECT_RATE);

void update();
void set_leds();

/////////////////////////////////////////////////////////////
// Sketch Entry Point
/////////////////////////////////////////////////////////////
void setup() {
  DEBUG_INIT()

  // Setup LoRa
  DEBUG_PRINTLN(F("Setting up LoRa Client"))
  if (!driver.init()) {
    DEBUG_PRINTLN(F("Error initializing Driver..."))
    exit(-1);
  }

  driver.setFrequency(DEFAULT_LORA_FREQ);
  driver.setCADTimeout(DEFAULT_LORA_CAD_TIMEOUT);

  DEBUG_PRINTLN(F("LoRa Client Setup Complete"))

  // LED
  led_info.transition_time = DEFAULT_TRANSITION_TIME;
  led_info.color1 = DEFAULT_COLOR1;
  led_info.color2 = DEFAULT_COLOR2;
  led_info.animation  = DEFAULT_STYLE;
  led_info.effect_rate = DEFAULT_EFFECT_RATE;

  leds.init();
  set_leds();
  lastLEDUpdate = millis();
}

/////////////////////////////////////////////////////////////
// Sketch Main Loop
/////////////////////////////////////////////////////////////
void loop() {
  if (driver.available()) {
    update();
  }

  if (millis() > lastLEDUpdate + REFRESH_RATE_MS) {
    leds.loop();
    FastLED.show();
    lastLEDUpdate = millis();
  }
}

/////////////////////////////////////////////////////////////
// Helpers
/////////////////////////////////////////////////////////////

void update() {
  XMasPacket pkt;

  if (!driver.available()) {
    return;
  }

  recv_packet(driver, &pkt);

  led_info = pkt.data;

  set_leds();
};

void set_leds() {
  if ((led_info.color1 != leds.getColor1()) ||
      (led_info.color1 != leds.getColor2()) ||
      (led_info.animation  != leds.animation)) {
    leds.setAlternating(led_info.color1, led_info.color2);
  }

  leds.updateColors(led_info.color1, led_info.color2);
  leds.animation = led_info.animation;
  leds.effect_rate = led_info.effect_rate;
  leds.transition_time = led_info.transition_time;
}