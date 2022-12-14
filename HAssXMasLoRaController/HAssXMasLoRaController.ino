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
#include <ESP8266WiFi.h>
#include <ArduinoHA.h>

#include <RH_RF95.h>
#include <RHReliableDatagram.h>

#include "secrets.h"

#include "XMasComms.h"

/////////////////////////////////////////////////////////////
// Debug Setup
/////////////////////////////////////////////////////////////

#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
  #define DEBUG_INIT() Serial.begin(115200);
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

#define MINIMUM_TRANSITION_TIME 50

#define VERSION            "1.0.0-Alpha"
#define DEVICE_NAME        "ERISC_Christmas_Lights"
#define DEVICE_MODEL       "ERISC_XMAS_Light_Controller"
#define DEVICE_MANUFACTURE "EmbeddedRISC"

/////////////////////////////////////////////////////////////
// HAss Definitions
/////////////////////////////////////////////////////////////

WiFiClient client;
HADevice   device;
HAMqtt     mqtt(client, device, 24);

HANumber transition_time("led_transition_time");

HALight led_color1("led_color1", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HALight led_color2("led_color2", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HASelect led_style("led_style");
HANumber led_effect_rate("led_effect_rate");

void connect_wifi();
void setup_ha_sensors();
void setup_ha_device();
void begin_mqtt();
void start_ha();

void init_set_ha();

void onStyleSelectCommand(int8_t idx, HASelect* sender);
void onTransitionTimeCommand(HANumeric number, HANumber* sender);
void onEffectRateCommand(HANumeric number, HANumber* sender);
void onLightStateCommand(bool state, HALight* sender);
void onLightBrightnessCommand(uint8_t brightness, HALight* sender);
void onLightRGBColorCommand(HALight::RGBColor color, HALight* sender);

CRGB haLight2CRGB(HALight &light);

/////////////////////////////////////////////////////////////
// LoRa Defintions
/////////////////////////////////////////////////////////////

#define LORA_CSN_PIN D8
#define LORA_IRQ_PIN D1

RH_RF95 driver(LORA_CSN_PIN, LORA_IRQ_PIN);

XMasPacket pkt;

void handle_clients();
void handle_updates();

/////////////////////////////////////////////////////////////
// Control Variables
/////////////////////////////////////////////////////////////

bool send_update;

/////////////////////////////////////////////////////////////
// Sketch Entry Point
/////////////////////////////////////////////////////////////
void setup() {
  DEBUG_INIT()

  // Setup LoRa
  DEBUG_PRINTLN("Setting up LoRa Server")
  if (!driver.init()) {
    DEBUG_PRINTLN("Error initializing Driver...")
    exit(-1);
  }

  driver.setFrequency(DEFAULT_LORA_FREQ);
  driver.setCADTimeout(DEFAULT_LORA_CAD_TIMEOUT);

  DEBUG_PRINTLN("LoRa Server Initialization Complete")

  // Setup HA
  start_ha();

  // Initialize control variable
  send_update = false;
}

/////////////////////////////////////////////////////////////
// Sketch Main Loop
/////////////////////////////////////////////////////////////
void loop() {
  mqtt.loop();

  handle_clients();

  handle_updates();
}

/////////////////////////////////////////////////////////////
// HAss Helper Functions
/////////////////////////////////////////////////////////////

/**
 * Connect the Board to WiFi
 */
void connect_wifi() {
  // Setup and Connect to the WiFi
  byte mac[WL_MAC_ADDR_LENGTH];
  WiFi.macAddress(mac);
  device.setUniqueId(mac, sizeof(mac));

  WiFi.begin(MY_SSID, MY_WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    DEBUG_PRINTLN("Trying to connect to WiFi")
    delay(500);
  }

  DEBUG_PRINTLN("Wifi Connected!")
}


/**
 * Setup all of the sensors (or entities) connected to the HA Device
 */
void setup_ha_sensors() {
  HALight::RGBColor default_color1_ha, default_color2_ha;
  CRGB default_color1, default_color2;

  default_color1 = DEFAULT_COLOR1;
  default_color2 = DEFAULT_COLOR2;

  default_color1_ha.red   = default_color1.red;
  default_color1_ha.green = default_color1.green;
  default_color1_ha.blue  = default_color1.blue;

  default_color2_ha.red   = default_color2.red;
  default_color2_ha.green = default_color2.green;
  default_color2_ha.blue  = default_color2.blue;

  // Global Settings
  transition_time.setName("LED Transition Time");
  transition_time.setUnitOfMeasurement("ms");
  transition_time.setMin(MINIMUM_TRANSITION_TIME);
  transition_time.setMax(4000);
  transition_time.setState(DEFAULT_TRANSITION_TIME);
  transition_time.onCommand(onTransitionTimeCommand);

  led_color1.setName("LED Color1");
  led_color1.onStateCommand(onLightStateCommand);
  led_color1.onBrightnessCommand(onLightBrightnessCommand);
  led_color1.onRGBColorCommand(onLightRGBColorCommand);
  led_color1.setState(DEFAULT_LED_STATE);
  led_color1.setBrightness(DEFAULT_BRIGHTNESS);
  led_color1.setRGBColor(default_color1_ha);

  led_color2.setName("LED Color2");
  led_color2.onStateCommand(onLightStateCommand);
  led_color2.onBrightnessCommand(onLightBrightnessCommand);
  led_color2.onRGBColorCommand(onLightRGBColorCommand);
  led_color2.setState(DEFAULT_LED_STATE);
  led_color2.setBrightness(DEFAULT_BRIGHTNESS);
  led_color2.setRGBColor(default_color2_ha);

  led_style.setName("Lighting Style");
  led_style.setOptions("simple;twinkle;fade_between");
  led_style.onCommand(onStyleSelectCommand);
  led_style.setState(DEFAULT_STYLE);

  led_effect_rate.setName("Light Effect Rate");
  led_effect_rate.onCommand(onEffectRateCommand);
  led_effect_rate.setState(DEFAULT_EFFECT_RATE);
}


/**
 * Setup the home assistant "device"
 */
void setup_ha_device() {
  device.setName(DEVICE_NAME);
  device.setSoftwareVersion(VERSION);
  device.setManufacturer(DEVICE_MANUFACTURE);
  device.setModel(DEVICE_MODEL);
  device.enableSharedAvailability();
  device.enableLastWill();
}


/**
 * Start the HAss MQTT Broker Connection
 */
void begin_mqtt() {
  mqtt.onConnected(init_set_ha);
  mqtt.begin(MQTT_IP_ADDR, MQTT_USER_NAME, MQTT_USER_PWD);
}


void init_set_ha() {
  HALight::RGBColor default_color1_ha, default_color2_ha;
  CRGB default_color1, default_color2;

  default_color1 = DEFAULT_COLOR1;
  default_color2 = DEFAULT_COLOR2;

  default_color1_ha.red   = default_color1.red;
  default_color1_ha.green = default_color1.green;
  default_color1_ha.blue  = default_color1.blue;
  default_color1_ha.isSet = true; 

  default_color2_ha.red   = default_color2.red;
  default_color2_ha.green = default_color2.green;
  default_color2_ha.blue  = default_color2.blue;
  default_color2_ha.isSet = true; 


  DEBUG_PRINTLN("HAss Connected!")

  transition_time.setState(DEFAULT_TRANSITION_TIME, true);
  led_color1.setState(DEFAULT_LED_STATE, true);
  led_color1.setBrightness(DEFAULT_BRIGHTNESS, true);
  led_color1.setRGBColor(default_color1_ha, true);
  led_color2.setState(DEFAULT_LED_STATE, true);
  led_color2.setBrightness(DEFAULT_BRIGHTNESS, true);
  led_color2.setRGBColor(default_color2_ha, true);
  led_style.setState(DEFAULT_STYLE, true);
  led_effect_rate.setState(DEFAULT_EFFECT_RATE, true);
}


/**
 * Walk the the entire process of starting a HAss Device
 */
void start_ha() {
  connect_wifi();
  setup_ha_sensors();
  setup_ha_device();
  begin_mqtt();
}

/////////////////////////////////////////////////////////////
// HAss CallBack Functions
/////////////////////////////////////////////////////////////

void onStyleSelectCommand(int8_t idx, HASelect* sender) {
  switch(idx) {
    case SIMPLE :
      sender->setState(idx);
      break;
    case TWINKLE :
      sender->setState(idx);
      break;
    case FADE_BETWEEN :
      sender->setState(idx);
      break;
    default:
      DEBUG_PRINTLN("Invalid Style Type!")
      sender->setState(SIMPLE);
      break;
  }

  send_update = true;
}

void onTransitionTimeCommand(HANumeric number, HANumber* sender) {
  sender->setState(number);

  send_update = true;
}

void onEffectRateCommand(HANumeric number, HANumber* sender) {
  sender->setState(number);

  send_update = true;
}

void onLightStateCommand(bool state, HALight* sender) {
  sender->setState(state);

  send_update = true;
}

void onLightBrightnessCommand(uint8_t brightness, HALight* sender) {
  sender->setBrightness(brightness);

  send_update = true;
}

void onLightRGBColorCommand(HALight::RGBColor color, HALight* sender) {
  sender->setRGBColor(color);

  send_update = true;
}

/////////////////////////////////////////////////////////////
// LoRa Helper Definitions
/////////////////////////////////////////////////////////////

void handle_clients() {
  uint8_t from;

  if (!driver.available()) {
    return;
  }

  if (recv_packet(driver, &pkt)) {}
}


void handle_updates() {
  LedInfo led_info;
  XMasPacket pkt;

  if (!send_update) {
    return;
  }

  led_info.transition_time = transition_time.getCurrentState().toInt16();
  led_info.color1 = haLight2CRGB(led_color1);
  led_info.color2 = haLight2CRGB(led_color2);
  led_info.effect_rate = led_effect_rate.getCurrentState().toInt16();
  led_info.animation = (AnimationType) led_style.getCurrentState();

  pkt.data = led_info;

  DEBUG_PRINTLN("Sending LED Info Packet:")

  send_packet(driver, &pkt);

  send_update = false;
}


CRGB haLight2CRGB(HALight &light) {
  CRGB ret_val;

  if (light.getCurrentState()) {
    ret_val.red = scale8(light.getCurrentRGBColor().red, dim8_raw(light.getCurrentBrightness()));
    ret_val.green = scale8(light.getCurrentRGBColor().green, dim8_raw(light.getCurrentBrightness()));
    ret_val.blue = scale8(light.getCurrentRGBColor().blue, dim8_raw(light.getCurrentBrightness()));
  } else {
    ret_val = CRGB::Black;
  }

  return ret_val;
}