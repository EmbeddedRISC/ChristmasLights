/*************************************************************************
 * Arduino Code for a Home-Assistant Arduino WS2811 Christmas Lights
 * Copyright (C) 2022  Benjamin Davis
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

// Board Definitions

//#define NODE_MCU_BOARD

// // Needed to make NodeMCU and FastLED play nicely
// #define FASTLED_INTERRUPT_RETRY_COUNT 0
// #define FASTLED_ESP8266_RAW_PIN_ORDER

/////////////////////////////////////////////////////////////
// Includes
/////////////////////////////////////////////////////////////

#ifdef NODE_MCU_BOARD
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>
#endif

#include "FastLED.h"

#include "secrets.h"
#include "LedChain.h"

/////////////////////////////////////////////////////////////
// Configuration
/////////////////////////////////////////////////////////////

#define SERIAL_DEBUG

#define VERSION            "1.0.0-Alpha"
#define DEVICE_NAME        "ERISC_Christmas_Lights"
#define DEVICE_MODEL       "ERISC_XMAS_Light_Controller"
#define DEVICE_MANUFACTURE "EmbeddedRISC"

#define NUM_LED_PINS 1
#define LEDS_PER_PIN 50
// const int LED_PINS[] = {D0, D1, D2, D3};

/////////////////////////////////////////////////////////////
// Definitions
/////////////////////////////////////////////////////////////

// Global Definitions for HAss Connection Stuff
#ifdef NODE_MCU_BOARD
WiFiClient client;
HADevice device;
HAMqtt mqtt(client, device);
#endif

// Gloabl Definitions for the sensors

// Global Definitions for the LEDS
CRGB raw_leds[LEDS_PER_PIN];
SmartLED leds[LEDS_PER_PIN];

#ifdef NODE_MCU_BOARD
  #define LED_BASE_PIN 16
#else
  #define LED_BASE_PIN 2
#endif


/////////////////////////////////////////////////////////////
// Sketch Entry Point
/////////////////////////////////////////////////////////////
void setup() {

  #ifdef SERIAL_DEBUG
    Serial.begin(500000);
  #endif

  // Configure the LED
  FastLED.addLeds<WS2811, LED_BASE_PIN>(raw_leds, LEDS_PER_PIN);

  for (int i = 0; i < LEDS_PER_PIN; i++) {
    leds[i].led = raw_leds + i;
    leds[i].set(CRGB::Green);
  }

  FastLED.show();

  // Configure the NODE MCU Board it using it
  #ifdef NODE_MCU_BOARD
  node_setup();
  #endif

  // Use Random noise on analog pin for randomness
  randomSeed(analogRead(A0));
}

// Setup helpers
#ifdef NODE_MCU_BOARD
void node_setup() {

  // Setup and Connect to the WiFi
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  WiFi.begin(MY_SSID, MY_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    #ifdef SERIAL_DEBUG
      Serial.println("Trying to connect to WiFi");
    #endif
    delay(500);
  }

  #ifdef SERIAL_DEBUG
    Serial.println("WiFi Connected");
  #endif

  // Setup the device information
  device.setName(DEVICE_NAME);
  device.setSoftwareVersion(VERSION);
  device.setManufacturer(DEVICE_MANUFACTURE);
  device.setModel(DEVICE_MODEL);
  device.enableSharedAvailability();
  device.enableLastWill();

  // Setup the HAss Sensor Defaults
  // TODO:

  // Connect to the MQTT Server
  // mqtt.begin(MQTT_IP_ADDR, MQTT_USER_NAME, MQTT_USER_PWD);
}
#endif

/////////////////////////////////////////////////////////////
// Sketch Main Loop
/////////////////////////////////////////////////////////////
void loop() {
  // mqtt.loop();
  for (int i = 0; i < LEDS_PER_PIN; i++) {
    leds[i].update();
  }
  if (random(0,3) == 0) {
    int idx = random(0, LEDS_PER_PIN);
    if (!leds[idx].isTransitioning()) {
      leds[idx].set(CRGB::Red);
      leds[idx].transition(CRGB::Green, 100);
    }
  }
  // if (random(0,2)) {
  //   int idx = random(0,50);
  //   leds[idx].set(CRGB::Green);
  //   // leds[idx].transition(CRGB::Green, 100);
  // }
  FastLED.show();
  delay(100);
}

/////////////////////////////////////////////////////////////
// Helper Functions
/////////////////////////////////////////////////////////////