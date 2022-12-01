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

#include <Wire.h>

#include "secrets.h"

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

#define MINIMUM_TRANSITION_TIME 50

#define VERSION            "1.0.0-Alpha"
#define DEVICE_NAME        "ERISC_Christmas_Lights"
#define DEVICE_MODEL       "ERISC_XMAS_Light_Controller"
#define DEVICE_MANUFACTURE "EmbeddedRISC"

#define SDA_PIN D2
#define SCL_PIN D1

/////////////////////////////////////////////////////////////
// HAss Definitions
/////////////////////////////////////////////////////////////

WiFiClient client;
HADevice   device;
HAMqtt     mqtt(client, device, 24);

HANumber transition_time("transition_time");

HALight garage_color1("garage_color1", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HALight garage_color2("garage_color2", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HASelect garage_style("garage_style");
HANumber garage_effect_rate("garage_effect_rate");

HALight door_color1("door_color1", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HALight door_color2("door_color2", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HASelect door_style("door_style");
HANumber door_effect_rate("door_effect_rate");

HALight  roof_color1("roof_color1", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HALight  roof_color2("roof_color2", HALight::BrightnessFeature |
                                        HALight::RGBFeature);
HASelect roof_style("roof_style");
HANumber roof_effect_rate("roof_effect_rate");

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

/////////////////////////////////////////////////////////////
// I2C Helper Definitions
/////////////////////////////////////////////////////////////

bool update_transition_time;
bool update_garage_color1;
bool update_garage_color2;
bool update_garage_style;
bool update_garage_effect_rate;
bool update_door_color1;
bool update_door_color2;
bool update_door_style;
bool update_door_effect_rate;
bool update_roof_color1;
bool update_roof_color2;
bool update_roof_style;
bool update_roof_effect_rate;

void handle_updates();

/////////////////////////////////////////////////////////////
// Sketch Entry Point
/////////////////////////////////////////////////////////////
void setup() {

  DEBUG_INIT()

  // Setup the i2c bus
  Wire.begin(SDA_PIN, SCL_PIN);

  update_transition_time = true;
  update_garage_color1 = true;
  update_garage_color2 = true;
  update_garage_style = true;
  update_garage_effect_rate = true;
  update_door_color1 = true;
  update_door_color2 = true;
  update_door_style = true;
  update_door_effect_rate = true;
  update_roof_color1 = true;
  update_roof_color2 = true;
  update_roof_style = true;
  update_roof_effect_rate = true;

  // Setup HA
  start_ha();
}

/////////////////////////////////////////////////////////////
// Sketch Main Loop
/////////////////////////////////////////////////////////////
void loop() {
  mqtt.loop();

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

  // Garage
  garage_color1.setName("Garage LED Color1");
  garage_color1.onStateCommand(onLightStateCommand);
  garage_color1.onBrightnessCommand(onLightBrightnessCommand);
  garage_color1.onRGBColorCommand(onLightRGBColorCommand);
  garage_color1.setState(DEFAULT_LED_STATE);
  garage_color1.setBrightness(DEFAULT_BRIGHTNESS);
  garage_color1.setRGBColor(default_color1_ha);

  garage_color2.setName("Garage LED Color2");
  garage_color2.onStateCommand(onLightStateCommand);
  garage_color2.onBrightnessCommand(onLightBrightnessCommand);
  garage_color2.onRGBColorCommand(onLightRGBColorCommand);
  garage_color2.setState(DEFAULT_LED_STATE);
  garage_color2.setBrightness(DEFAULT_BRIGHTNESS);
  garage_color2.setRGBColor(default_color2_ha);

  garage_style.setName("Garage Lighting Style");
  garage_style.setOptions("off;simple;twinkle;fade_between");
  garage_style.onCommand(onStyleSelectCommand);
  garage_style.setState(DEFAULT_STYLE);

  garage_effect_rate.setName("Garage Light Effect Rate");
  garage_effect_rate.onCommand(onEffectRateCommand);
  garage_effect_rate.setState(DEFAULT_EFFECT_RATE);

  // DOOR
  door_color1.setName("Door LED Color1");
  door_color1.onStateCommand(onLightStateCommand);
  door_color1.onBrightnessCommand(onLightBrightnessCommand);
  door_color1.onRGBColorCommand(onLightRGBColorCommand);
  door_color1.setState(DEFAULT_LED_STATE);
  door_color1.setBrightness(DEFAULT_BRIGHTNESS);
  door_color1.setRGBColor(default_color1_ha);

  door_color2.setName("Door LED Color2");
  door_color2.onStateCommand(onLightStateCommand);
  door_color2.onBrightnessCommand(onLightBrightnessCommand);
  door_color2.onRGBColorCommand(onLightRGBColorCommand);
  door_color2.setState(DEFAULT_LED_STATE);
  door_color2.setBrightness(DEFAULT_BRIGHTNESS);
  door_color2.setRGBColor(default_color2_ha);

  door_style.setName("Door Lighting Style");
  door_style.setOptions(XMasComms::LED_STYLES);
  door_style.onCommand(onStyleSelectCommand);
  door_style.setState(DEFAULT_STYLE);

  door_effect_rate.setName("Door Light Effect Rate");
  door_effect_rate.onCommand(onEffectRateCommand);
  door_effect_rate.setState(DEFAULT_EFFECT_RATE);

  // ROOF
  roof_color1.setName("Roof LED Color1");
  roof_color1.onStateCommand(onLightStateCommand);
  roof_color1.onBrightnessCommand(onLightBrightnessCommand);
  roof_color1.onRGBColorCommand(onLightRGBColorCommand);
  roof_color1.setState(DEFAULT_LED_STATE);
  roof_color1.setBrightness(DEFAULT_BRIGHTNESS);
  roof_color1.setRGBColor(default_color1_ha);

  roof_color2.setName("Roof LED Color2");
  roof_color2.onStateCommand(onLightStateCommand);
  roof_color2.onBrightnessCommand(onLightBrightnessCommand);
  roof_color2.onRGBColorCommand(onLightRGBColorCommand);
  roof_color2.setState(DEFAULT_LED_STATE);
  roof_color2.setBrightness(DEFAULT_BRIGHTNESS);
  roof_color2.setRGBColor(default_color2_ha);

  roof_style.setName("Roof Lighting Style");
  roof_style.setOptions(XMasComms::LED_STYLES);
  roof_style.onCommand(onStyleSelectCommand);
  roof_style.setState(DEFAULT_STYLE);

  roof_effect_rate.setName("Roof Light Effect Rate");
  roof_effect_rate.onCommand(onEffectRateCommand);
  roof_effect_rate.setState(DEFAULT_EFFECT_RATE);
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
  DEBUG_PRINTLN("HAss Connected!")

  transition_time.setState(DEFAULT_TRANSITION_TIME);

  // Garage
  garage_color1.setState(garage_color1.getCurrentState());
  garage_color1.setBrightness(garage_color1.getCurrentBrightness());
  garage_color1.setRGBColor(garage_color1.getCurrentRGBColor());
  garage_color2.setState(garage_color2.getCurrentState());
  garage_color2.setBrightness(garage_color2.getCurrentBrightness());
  garage_color2.setRGBColor(garage_color2.getCurrentRGBColor());
  garage_style.setState(garage_style.getCurrentState());
  garage_effect_rate.setState(garage_effect_rate.getCurrentState());

  // DOOR
  door_color1.setState(door_color1.getCurrentState());
  door_color1.setBrightness(door_color1.getCurrentBrightness());
  door_color1.setRGBColor(door_color1.getCurrentRGBColor());
  door_color2.setState(door_color2.getCurrentState());
  door_color2.setBrightness(door_color2.getCurrentBrightness());
  door_color2.setRGBColor(door_color2.getCurrentRGBColor());
  door_style.setState(door_style.getCurrentState());
  door_effect_rate.setState(door_effect_rate.getCurrentState());

  // ROOF
  roof_color1.setState(roof_color1.getCurrentState());
  roof_color1.setBrightness(roof_color1.getCurrentBrightness());
  roof_color1.setRGBColor(roof_color1.getCurrentRGBColor());
  roof_color2.setState(roof_color2.getCurrentState());
  roof_color2.setBrightness(roof_color2.getCurrentBrightness());
  roof_color2.setRGBColor(roof_color2.getCurrentRGBColor());
  roof_style.setState(roof_style.getCurrentState());
  roof_effect_rate.setState(roof_effect_rate.getCurrentState());
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
    case XMasComms::OFF :
      sender->setState(idx);
      break;
    case XMasComms::SIMPLE :
      sender->setState(idx);
      break;
    case XMasComms::TWINKLE :
      sender->setState(idx);
      break;
    case XMasComms::FADE_BETWEEN :
      sender->setState(idx);
      break;
    default:
      DEBUG_PRINTLN("Invalid Style Type!")
      sender->setState(-1);
      break;
  }

  if (sender->uniqueId() == garage_style.uniqueId()) {
    update_garage_style = true;
  }
  else if (sender->uniqueId() == door_style.uniqueId()) {
    update_door_style = true;
  }
  else if (sender->uniqueId() == roof_style.uniqueId()) {
    update_roof_style = true;
  }
}

void onTransitionTimeCommand(HANumeric number, HANumber* sender) {
  sender->setState(number);

  update_transition_time = true;
}

void onEffectRateCommand(HANumeric number, HANumber* sender) {
  sender->setState(number);

  if (sender->uniqueId() == garage_effect_rate.uniqueId()) {
    update_garage_effect_rate = true;
  }
  else if (sender->uniqueId() == door_effect_rate.uniqueId()) {
    update_door_effect_rate = true;
  }
  else if (sender->uniqueId() == roof_effect_rate.uniqueId()) {
    update_roof_effect_rate = true;
  }
}

void onLightStateCommand(bool state, HALight* sender) {
  sender->setState(state);

  if (sender->uniqueId() == garage_color1.uniqueId()) {
    update_garage_color1 = true;
  }
  else if (sender->uniqueId() == garage_color2.uniqueId()) {
    update_garage_color2 = true;
  }
  else if (sender->uniqueId() == door_color1.uniqueId()) {
    update_door_color1 = true;
  }
  else if (sender->uniqueId() == door_color2.uniqueId()) {
    update_door_color2 = true;
  }
  else if (sender->uniqueId() == roof_color1.uniqueId()) {
    update_roof_color1 = true;
  }
  else if (sender->uniqueId() == roof_color2.uniqueId()) {
    update_roof_color2 = true;
  }
}

void onLightBrightnessCommand(uint8_t brightness, HALight* sender) {
  sender->setBrightness(brightness);

  if (sender->uniqueId() == garage_color1.uniqueId()) {
    update_garage_color1 = true;
  }
  else if (sender->uniqueId() == garage_color2.uniqueId()) {
    update_garage_color2 = true;
  }
  else if (sender->uniqueId() == door_color1.uniqueId()) {
    update_door_color1 = true;
  }
  else if (sender->uniqueId() == door_color2.uniqueId()) {
    update_door_color2 = true;
  }
  else if (sender->uniqueId() == roof_color1.uniqueId()) {
    update_roof_color1 = true;
  }
  else if (sender->uniqueId() == roof_color2.uniqueId()) {
    update_roof_color2 = true;
  }
}

void onLightRGBColorCommand(HALight::RGBColor color, HALight* sender) {
  sender->setRGBColor(color);

  if (sender->uniqueId() == garage_color1.uniqueId()) {
    update_garage_color1 = true;
  }
  else if (sender->uniqueId() == garage_color2.uniqueId()) {
    update_garage_color2 = true;
  }
  else if (sender->uniqueId() == door_color1.uniqueId()) {
    update_door_color1 = true;
  }
  else if (sender->uniqueId() == door_color2.uniqueId()) {
    update_door_color2 = true;
  }
  else if (sender->uniqueId() == roof_color1.uniqueId()) {
    update_roof_color1 = true;
  }
  else if (sender->uniqueId() == roof_color2.uniqueId()) {
    update_roof_color2 = true;
  }
}

/////////////////////////////////////////////////////////////
// I2C Helper Definitions
/////////////////////////////////////////////////////////////

void _update_transition_time(HANumber &tt) {
  XMasComms::TransitionTimePacket pkt;

  pkt.data.time = (unsigned short)tt.getCurrentState().toUInt16();

  Wire.beginTransmission(XMasComms::SLAVE_I2C_ADDR);
  Wire.write(XMasComms::TRANSITION_TIME);
  Wire.write(pkt.buf, sizeof(pkt));
  Wire.endTransmission();
}

void _update_color(XMasComms::XMasControllerField fld, HALight &light) {
  XMasComms::ColorPacket pkt;

  pkt.data.rgb.r = light.getCurrentRGBColor().red;
  pkt.data.rgb.g = light.getCurrentRGBColor().green;
  pkt.data.rgb.b = light.getCurrentRGBColor().blue;
  pkt.data.brightness = light.getCurrentBrightness();
  pkt.data.state      = light.getCurrentState();

  Wire.beginTransmission(XMasComms::SLAVE_I2C_ADDR);
  Wire.write(fld);
  Wire.write(pkt.buf, sizeof(pkt));
  Wire.endTransmission();
}

void _update_style(XMasComms::XMasControllerField fld, HASelect &sty) {
  XMasComms::StylePacket pkt;

  pkt.data.style = (XMasComms::LEDStyles)sty.getCurrentState();

  Wire.beginTransmission(XMasComms::SLAVE_I2C_ADDR);
  Wire.write(fld);
  Wire.write(pkt.buf, sizeof(pkt));
  Wire.endTransmission();
}

void _update_effect_rate(XMasComms::XMasControllerField fld, HANumber &rate) {
  XMasComms::EffectRatePacket pkt;

  pkt.data.rate = (unsigned short)rate.getCurrentState().toUInt16();

  Wire.beginTransmission(XMasComms::SLAVE_I2C_ADDR);
  Wire.write(fld);
  Wire.write(pkt.buf, sizeof(pkt));
  Wire.endTransmission();
}

void handle_updates() {
  if (update_transition_time) {
    _update_transition_time(transition_time);
    update_transition_time = false;
  }

  if (update_garage_color1) {
    _update_color(XMasComms::GARAGE_COLOR1, garage_color1);
    update_garage_color1 = false;
  }

  if (update_garage_color2) {
    _update_color(XMasComms::GARAGE_COLOR2, garage_color2);
    update_garage_color2 = false;
  }

  if (update_garage_style) {
    _update_style(XMasComms::GARAGE_STYLE, garage_style);
    update_garage_style = false;
  }

  if (update_garage_effect_rate) {
    _update_effect_rate(XMasComms::GARAGE_EFFECT_RATE, garage_effect_rate);
    update_garage_effect_rate = false;
  }

  if (update_door_color1) {
    _update_color(XMasComms::DOOR_COLOR1, door_color1);
    update_door_color1 = false;
  }

  if (update_door_color2) {
    _update_color(XMasComms::DOOR_COLOR2, door_color2);
    update_door_color2 = false;
  }

  if (update_door_style) {
    _update_style(XMasComms::DOOR_STYLE, door_style);
    update_door_style = false;
  }

  if (update_door_effect_rate) {
    _update_effect_rate(XMasComms::DOOR_EFFECT_RATE, door_effect_rate);
    update_door_effect_rate = false;
  }

  if (update_roof_color1) {
    _update_color(XMasComms::ROOF_COLOR1, roof_color1);
    update_roof_color1 = false;
  }

  if (update_roof_color2) {
    _update_color(XMasComms::ROOF_COLOR2, roof_color2);
    update_roof_color2 = false;
  }

  if (update_roof_style) {
    _update_style(XMasComms::ROOF_STYLE, roof_style);
     update_roof_style = false;
  }

  if (update_roof_effect_rate) {
    _update_effect_rate(XMasComms::ROOF_EFFECT_RATE, roof_effect_rate);
    update_roof_effect_rate = false;
  }
}