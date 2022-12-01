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
#include "XMasComms.h"
#include <Wire.h>

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

#define GARAGE_LED_COUNT 150
#define DOOR_LED_COUNT   50
#define ROOF_LED_COUNT   100

#define GARAGE_LED_PIN 2
#define DOOR_LED_PIN   3
#define ROOF_LED_PIN   4

#define REFRESH_RATE_MS 50

/////////////////////////////////////////////////////////////
// Field Definition
/////////////////////////////////////////////////////////////
struct Fields {
  XMasComms::TransitionTime transition_time;
  XMasComms::Color          garage_color1;
  XMasComms::Color          garage_color2;
  XMasComms::Style          garage_style;
  XMasComms::EffectRate     garage_effect_rate;
  XMasComms::Color          door_color1;
  XMasComms::Color          door_color2;
  XMasComms::Style          door_style;
  XMasComms::EffectRate     door_effect_rate;
  XMasComms::Color          roof_color1;
  XMasComms::Color          roof_color2;
  XMasComms::Style          roof_style;
  XMasComms::EffectRate     roof_effect_rate;
};

Fields cur_fields;
Fields prev_fields;

void init_fields() {
  cur_fields.transition_time.time     = DEFAULT_TRANSITION_TIME;
  cur_fields.garage_color1.rgb        = DEFAULT_COLOR1;
  cur_fields.garage_color1.brightness = DEFAULT_BRIGHTNESS;
  cur_fields.garage_color1.state      = DEFAULT_LED_STATE;
  cur_fields.garage_color2.rgb        = DEFAULT_COLOR2;
  cur_fields.garage_color2.brightness = DEFAULT_BRIGHTNESS;
  cur_fields.garage_color2.state      = DEFAULT_LED_STATE;
  cur_fields.garage_style.style       = DEFAULT_STYLE;
  cur_fields.garage_effect_rate.rate  = DEFAULT_EFFECT_RATE;
  cur_fields.door_color1.rgb          = DEFAULT_COLOR1;
  cur_fields.door_color1.brightness   = DEFAULT_BRIGHTNESS;
  cur_fields.door_color1.state        = DEFAULT_LED_STATE;
  cur_fields.door_color2.rgb          = DEFAULT_COLOR2;
  cur_fields.door_color2.brightness   = DEFAULT_BRIGHTNESS;
  cur_fields.door_color2.state        = DEFAULT_LED_STATE;
  cur_fields.door_style.style         = DEFAULT_STYLE;
  cur_fields.door_effect_rate.rate    = DEFAULT_EFFECT_RATE;
  cur_fields.roof_color1.rgb          = DEFAULT_COLOR1;
  cur_fields.roof_color1.brightness   = DEFAULT_BRIGHTNESS;
  cur_fields.roof_color1.state        = DEFAULT_LED_STATE;
  cur_fields.roof_color2.rgb          = DEFAULT_COLOR2;
  cur_fields.roof_color2.brightness   = DEFAULT_BRIGHTNESS;
  cur_fields.roof_color2.state        = DEFAULT_LED_STATE;
  cur_fields.roof_style.style         = DEFAULT_STYLE;
  cur_fields.roof_effect_rate.rate    = DEFAULT_EFFECT_RATE;

  prev_fields.transition_time.time     = DEFAULT_TRANSITION_TIME;
  prev_fields.garage_color1.rgb        = DEFAULT_COLOR1;
  prev_fields.garage_color1.brightness = DEFAULT_BRIGHTNESS;
  prev_fields.garage_color1.state      = DEFAULT_LED_STATE;
  prev_fields.garage_color2.rgb        = DEFAULT_COLOR2;
  prev_fields.garage_color2.brightness = DEFAULT_BRIGHTNESS;
  prev_fields.garage_color2.state      = DEFAULT_LED_STATE;
  prev_fields.garage_style.style       = DEFAULT_STYLE;
  prev_fields.garage_effect_rate.rate  = DEFAULT_EFFECT_RATE;
  prev_fields.door_color1.rgb          = DEFAULT_COLOR1;
  prev_fields.door_color1.brightness   = DEFAULT_BRIGHTNESS;
  prev_fields.door_color1.state        = DEFAULT_LED_STATE;
  prev_fields.door_color2.rgb          = DEFAULT_COLOR2;
  prev_fields.door_color2.brightness   = DEFAULT_BRIGHTNESS;
  prev_fields.door_color2.state        = DEFAULT_LED_STATE;
  prev_fields.door_style.style         = DEFAULT_STYLE;
  prev_fields.door_effect_rate.rate    = DEFAULT_EFFECT_RATE;
  prev_fields.roof_color1.rgb          = DEFAULT_COLOR1;
  prev_fields.roof_color1.brightness   = DEFAULT_BRIGHTNESS;
  prev_fields.roof_color1.state        = DEFAULT_LED_STATE;
  prev_fields.roof_color2.rgb          = DEFAULT_COLOR2;
  prev_fields.roof_color2.brightness   = DEFAULT_BRIGHTNESS;
  prev_fields.roof_color2.state        = DEFAULT_LED_STATE;
  prev_fields.roof_style.style         = DEFAULT_STYLE;
  prev_fields.roof_effect_rate.rate    = DEFAULT_EFFECT_RATE;
}

void _debug_print_fields() {
  DEBUG_PRINT("transition_time.time:     ")
  DEBUG_PRINTLN(cur_fields.transition_time.time)
  DEBUG_PRINT("garage_color1.rgb:        ")
  DEBUG_PRINTLN(cur_fields.garage_color1.rgb)
  DEBUG_PRINT("garage_color1.brightness: ")
  DEBUG_PRINTLN(cur_fields.garage_color1.brightness)
  DEBUG_PRINT("garage_color1.state:      ")
  DEBUG_PRINTLN(cur_fields.garage_color1.state)
  DEBUG_PRINT("garage_color2.rgb:        ")
  DEBUG_PRINTLN(cur_fields.garage_color2.rgb)
  DEBUG_PRINT("garage_color2.brightness: ")
  DEBUG_PRINTLN(cur_fields.garage_color2.brightness)
  DEBUG_PRINT("garage_color2.state:      ")
  DEBUG_PRINTLN(cur_fields.garage_color2.state)
  DEBUG_PRINT("garage_style.style:       ")
  DEBUG_PRINTLN(cur_fields.garage_style.style)
  DEBUG_PRINT("garage_effect_rate.rate:  ")
  DEBUG_PRINTLN(cur_fields.garage_effect_rate.rate)
  DEBUG_PRINT("door_color1.rgb:          ")
  DEBUG_PRINTLN(cur_fields.door_color1.rgb)
  DEBUG_PRINT("door_color1.brightness:   ")
  DEBUG_PRINTLN(cur_fields.door_color1.brightness)
  DEBUG_PRINT("door_color1.state:        ")
  DEBUG_PRINTLN(cur_fields.door_color1.state)
  DEBUG_PRINT("door_color2.rgb:          ")
  DEBUG_PRINTLN(cur_fields.door_color2.rgb)
  DEBUG_PRINT("door_color2.brightness:   ")
  DEBUG_PRINTLN(cur_fields.door_color2.brightness)
  DEBUG_PRINT("door_color2.state:        ")
  DEBUG_PRINTLN(cur_fields.door_color2.state)
  DEBUG_PRINT("door_style.style:         ")
  DEBUG_PRINTLN(cur_fields.door_style.style)
  DEBUG_PRINT("door_effect_rate.rate:    ")
  DEBUG_PRINTLN(cur_fields.door_effect_rate.rate)
  DEBUG_PRINT("roof_color1.rgb:          ")
  DEBUG_PRINTLN(cur_fields.roof_color1.rgb)
  DEBUG_PRINT("roof_color1.brightness:   ")
  DEBUG_PRINTLN(cur_fields.roof_color1.brightness)
  DEBUG_PRINT("roof_color1.state:        ")
  DEBUG_PRINTLN(cur_fields.roof_color1.state)
  DEBUG_PRINT("roof_color2.rgb:          ")
  DEBUG_PRINTLN(cur_fields.roof_color2.rgb)
  DEBUG_PRINT("roof_color2.brightness:   ")
  DEBUG_PRINTLN(cur_fields.roof_color2.brightness)
  DEBUG_PRINT("roof_color2.state:        ")
  DEBUG_PRINTLN(cur_fields.roof_color2.state)
  DEBUG_PRINT("roof_style.style:         ")
  DEBUG_PRINTLN(cur_fields.roof_style.style)
  DEBUG_PRINT("roof_effect_rate.rate:    ")
  DEBUG_PRINTLN(cur_fields.roof_effect_rate.rate)
  DEBUG_PRINTLN()
  DEBUG_PRINTLN()
}

void receiveEvent(int bytes_read);
void requestEvent();

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
// LED Definition
/////////////////////////////////////////////////////////////

LedChain<GARAGE_LED_COUNT, GARAGE_LED_PIN> garage_leds(DEFAULT_TRANSITION_TIME, REFRESH_RATE_MS, DEFAULT_EFFECT_RATE);
LedChain<DOOR_LED_COUNT, DOOR_LED_PIN> door_leds(DEFAULT_TRANSITION_TIME, REFRESH_RATE_MS, DEFAULT_EFFECT_RATE);
LedChain<ROOF_LED_COUNT, ROOF_LED_PIN> roof_leds(DEFAULT_TRANSITION_TIME, REFRESH_RATE_MS, DEFAULT_EFFECT_RATE);

/////////////////////////////////////////////////////////////
// Sketch Entry Point
/////////////////////////////////////////////////////////////
void setup() {

  #ifdef SERIAL_DEBUG
    Serial.begin(500000);
  #endif

  // Setup the i2c bus
  Wire.begin(XMasComms::SLAVE_I2C_ADDR);
  Wire.onReceive(receiveEvent);
  Wire.onRequest(requestEvent);

  // Setup the fields
  init_fields();

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


  // Initial LED Setup
  garage_leds.init();
  garage_leds.setAlternating(DEFAULT_COLOR1, DEFAULT_COLOR2);

  door_leds.init();
  door_leds.setAlternating(DEFAULT_COLOR1, DEFAULT_COLOR2);

  roof_leds.init();
  roof_leds.setAlternating(DEFAULT_COLOR1, DEFAULT_COLOR2);

  FastLED.show();
}

/////////////////////////////////////////////////////////////
// Sketch Main Loop
/////////////////////////////////////////////////////////////
void loop() {
  garage_leds.loop();
  door_leds.loop();
  roof_leds.loop();

  FastLED.show();

  handle_updates();
  prev_fields = cur_fields;

  delay(REFRESH_RATE_MS);
}

/////////////////////////////////////////////////////////////
// I2C Functions
/////////////////////////////////////////////////////////////

void receiveEvent(int bytes_read) {
  uint8_t buf[sizeof(XMasComms::ColorPacket) + 1];

  DEBUG_PRINT("I2C Recv: ")
  DEBUG_PRINTLN(bytes_read)

  for (int i = 0; i < bytes_read; i++) {
    buf[i] = Wire.read();

    DEBUG_PRINT("\tByte ")
    DEBUG_PRINT(i)
    DEBUG_PRINT(": ")
    DEBUG_PRINTLN(String(buf[i], HEX))
  }

  DEBUG_PRINTLN(buf[0])

  switch((XMasComms::XMasControllerField) buf[0]) {
    case XMasComms::TRANSITION_TIME    :
      XMasComms::TransitionTimePacket tt_pkt;
      for (int i = 0; i < sizeof(XMasComms::TransitionTimePacket); i++) {
        tt_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.transition_time = tt_pkt.data;
      update_transition_time = true;
      break;

    case XMasComms::GARAGE_COLOR1      :
      XMasComms::ColorPacket gc1_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        gc1_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.garage_color1 = gc1_pkt.data;
      update_garage_color1 = true;
      break;

    case XMasComms::GARAGE_COLOR2      :
      XMasComms::ColorPacket gc2_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        gc2_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.garage_color2 = gc2_pkt.data;
      update_garage_color2 = true;
      break;

    case XMasComms::GARAGE_STYLE       :
      XMasComms::StylePacket gs_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        gs_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.garage_style = gs_pkt.data;
      update_garage_style = true;
      break;

    case XMasComms::GARAGE_EFFECT_RATE :
      XMasComms::EffectRatePacket ger_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        ger_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.garage_effect_rate = ger_pkt.data;
      update_garage_effect_rate = true;
      break;

    case XMasComms::DOOR_COLOR1        :
      XMasComms::ColorPacket dc1_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        dc1_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.door_color1 = dc1_pkt.data;
      update_door_color1 = true;
      break;

    case XMasComms::DOOR_COLOR2        :
      XMasComms::ColorPacket dc2_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        dc2_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.door_color2 = dc2_pkt.data;
      update_door_color2 = true;
      break;

    case XMasComms::DOOR_STYLE         :
      XMasComms::StylePacket ds_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        ds_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.door_style = ds_pkt.data;
      update_door_style = true;
      break;

    case XMasComms::DOOR_EFFECT_RATE   :
      XMasComms::EffectRatePacket der_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        der_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.door_effect_rate = der_pkt.data;
      update_door_effect_rate = true;
      break;

    case XMasComms::ROOF_COLOR1        :
      XMasComms::ColorPacket rc1_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        rc1_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.roof_color1 = rc1_pkt.data;
      update_roof_color1 = true;
      break;

    case XMasComms::ROOF_COLOR2        :
      XMasComms::ColorPacket rc2_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        rc2_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.roof_color2 = rc2_pkt.data;
      update_roof_color2 = true;
      break;

    case XMasComms::ROOF_STYLE         :
      XMasComms::StylePacket rs_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        rs_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.roof_style = rs_pkt.data;
      update_roof_style = true;
      break;

    case XMasComms::ROOF_EFFECT_RATE   :
      XMasComms::EffectRatePacket rer_pkt;
      for (int i = 0; i < sizeof(XMasComms::ColorPacket); i++) {
        rer_pkt.buf[i] = buf[i + 1];
      }
      cur_fields.roof_effect_rate = rer_pkt.data;
      update_roof_effect_rate = true;
      break;
  }
}


void requestEvent() {}

/////////////////////////////////////////////////////////////
// I2C Helper Definitions
/////////////////////////////////////////////////////////////

void _update_transition_time() {
  garage_leds.transition_time = cur_fields.transition_time.time;
  door_leds.transition_time = cur_fields.transition_time.time;
  roof_leds.transition_time = cur_fields.transition_time.time;
}

void _update_color(XMasComms::XMasControllerField fld) {
  CRGB c1, c2;

  switch (fld) {
    case XMasComms::GARAGE_COLOR1:
    case XMasComms::GARAGE_COLOR2:

      DEBUG_PRINTLN("Updating Garage Color")

      if (cur_fields.garage_color1.state) {
        c1.red = scale8(cur_fields.garage_color1.rgb.red, dim8_raw(cur_fields.garage_color1.brightness));
        c1.green = scale8(cur_fields.garage_color1.rgb.green, dim8_raw(cur_fields.garage_color1.brightness));
        c1.blue = scale8(cur_fields.garage_color1.rgb.blue, dim8_raw(cur_fields.garage_color1.brightness));
      } else {
        DEBUG_PRINTLN("Garage C1 off")
        c1 = CRGB::Black;
      }

      if (cur_fields.garage_color2.state) {
        c2.red = scale8(cur_fields.garage_color2.rgb.red, dim8_raw(cur_fields.garage_color2.brightness));
        c2.green = scale8(cur_fields.garage_color2.rgb.green, dim8_raw(cur_fields.garage_color2.brightness));
        c2.blue = scale8(cur_fields.garage_color2.rgb.blue, dim8_raw(cur_fields.garage_color2.brightness));
      } else {
        DEBUG_PRINTLN("Garage C2 off")
        c2 = CRGB::Black;
      }

      garage_leds.updateColors(c1, c2);

      break;

    case XMasComms::DOOR_COLOR1:
    case XMasComms::DOOR_COLOR2:

      DEBUG_PRINTLN("Updating Door Color")

      if (cur_fields.door_color1.state) {
        c1.red = scale8(cur_fields.door_color1.rgb.red, dim8_raw(cur_fields.door_color1.brightness));
        c1.green = scale8(cur_fields.door_color1.rgb.green, dim8_raw(cur_fields.door_color1.brightness));
        c1.blue = scale8(cur_fields.door_color1.rgb.blue, dim8_raw(cur_fields.door_color1.brightness));
      } else {
        DEBUG_PRINTLN("Door C1 off")
        c1 = CRGB::Black;
      }

      if (cur_fields.door_color2.state) {
        c2.red = scale8(cur_fields.door_color2.rgb.red, dim8_raw(cur_fields.door_color2.brightness));
        c2.green = scale8(cur_fields.door_color2.rgb.green, dim8_raw(cur_fields.door_color2.brightness));
        c2.blue = scale8(cur_fields.door_color2.rgb.blue, dim8_raw(cur_fields.door_color2.brightness));
      } else {
        DEBUG_PRINTLN("Door C2 off")
        c2 = CRGB::Black;
      }

      door_leds.updateColors(c1, c2);

      break;

    case XMasComms::ROOF_COLOR1:
    case XMasComms::ROOF_COLOR2:

      DEBUG_PRINTLN("Updating Roof Color")

      if (cur_fields.roof_color1.state) {
        c1.red = scale8(cur_fields.roof_color1.rgb.red, dim8_raw(cur_fields.roof_color1.brightness));
        c1.green = scale8(cur_fields.roof_color1.rgb.green, dim8_raw(cur_fields.roof_color1.brightness));
        c1.blue = scale8(cur_fields.roof_color1.rgb.blue, dim8_raw(cur_fields.roof_color1.brightness));
      } else {
        DEBUG_PRINTLN("Roof C1 off")
        c1 = CRGB::Black;
      }

      if (cur_fields.roof_color2.state) {
        c2.red = scale8(cur_fields.roof_color2.rgb.red, dim8_raw(cur_fields.roof_color2.brightness));
        c2.green = scale8(cur_fields.roof_color2.rgb.green, dim8_raw(cur_fields.roof_color2.brightness));
        c2.blue = scale8(cur_fields.roof_color2.rgb.blue, dim8_raw(cur_fields.roof_color2.brightness));
      } else {
        DEBUG_PRINTLN("Roof C2 off")
        c2 = CRGB::Black;
      }

      roof_leds.updateColors(c1, c2);
      break;

    default:
      DEBUG_PRINTLN("Invalid Controller Field passed to _update_color")
      break;
  }
}

void _update_style(XMasComms::XMasControllerField fld) {
  switch(fld) {
    case XMasComms::GARAGE_STYLE :
      DEBUG_PRINTLN("Updating GARAGE_STYLE")

      if (cur_fields.garage_style.style == XMasComms::OFF) {
        garage_leds.turnOff();
      }
      else if (prev_fields.garage_style.style == XMasComms::OFF) {
        garage_leds.turnOn();
      }

      if (cur_fields.garage_style.style == XMasComms::SIMPLE) {
        garage_leds.animation = SIMPLE;
      }
      else if (cur_fields.garage_style.style == XMasComms::TWINKLE) {
        garage_leds.animation = TWINKLE;
      }
      else if (cur_fields.garage_style.style == XMasComms::FADE_BETWEEN) {
        garage_leds.animation = FADE_BETWEEN;
      }

      break;
    case XMasComms::DOOR_STYLE :
      DEBUG_PRINTLN("Updating DOOR_STYLE")

      if (cur_fields.door_style.style == XMasComms::OFF) {
        door_leds.turnOff();
      }
      else if (prev_fields.door_style.style == XMasComms::OFF) {
        door_leds.turnOn();
      }

      if (cur_fields.door_style.style == XMasComms::SIMPLE) {
        door_leds.animation = SIMPLE;
      }
      else if (cur_fields.door_style.style == XMasComms::TWINKLE) {
        door_leds.animation = TWINKLE;
      }
      else if (cur_fields.door_style.style == XMasComms::FADE_BETWEEN) {
        door_leds.animation = FADE_BETWEEN;
      }

      break;
    case XMasComms::ROOF_STYLE :
      DEBUG_PRINTLN("Updating ROOF_STYLE")

      if (cur_fields.roof_style.style == XMasComms::OFF) {
        roof_leds.turnOff();
      }
      else if (prev_fields.roof_style.style == XMasComms::OFF) {
        roof_leds.turnOn();
      }

      if (cur_fields.roof_style.style == XMasComms::SIMPLE) {
        roof_leds.animation = SIMPLE;
      }
      else if (cur_fields.roof_style.style == XMasComms::TWINKLE) {
        roof_leds.animation = TWINKLE;
      }
      else if (cur_fields.roof_style.style == XMasComms::FADE_BETWEEN) {
        roof_leds.animation = FADE_BETWEEN;
      }

      break;

  }
}

void _update_effect_rate(XMasComms::XMasControllerField fld) {
  //TODO
}

void handle_updates() {
  if (update_transition_time) {
    _update_transition_time();
    update_transition_time = false;
  }

  if (update_garage_color1) {
    _update_color(XMasComms::GARAGE_COLOR1);
    update_garage_color1 = false;
  }

  if (update_garage_color2) {
    _update_color(XMasComms::GARAGE_COLOR2);
    update_garage_color2 = false;
  }

  if (update_garage_style) {
    _update_style(XMasComms::GARAGE_STYLE);
    update_garage_style = false;
  }

  if (update_garage_effect_rate) {
    _update_effect_rate(XMasComms::GARAGE_EFFECT_RATE);
    update_garage_effect_rate = false;
  }

  if (update_door_color1) {
    _update_color(XMasComms::DOOR_COLOR1);
    update_door_color1 = false;
  }

  if (update_door_color2) {
    _update_color(XMasComms::DOOR_COLOR1);
    update_door_color2 = false;
  }

  if (update_door_style) {
    _update_style(XMasComms::DOOR_STYLE);
    update_door_style = false;
  }

  if (update_door_effect_rate) {
    _update_effect_rate(XMasComms::DOOR_EFFECT_RATE);
    update_door_effect_rate = false;
  }

  if (update_roof_color1) {
    _update_color(XMasComms::ROOF_COLOR1);
    update_roof_color1 = false;
  }

  if (update_roof_color2) {
    _update_color(XMasComms::ROOF_COLOR2);
    update_roof_color2 = false;
  }

  if (update_roof_style) {
    _update_style(XMasComms::ROOF_STYLE);
     update_roof_style = false;
  }

  if (update_roof_effect_rate) {
    _update_effect_rate(XMasComms::ROOF_EFFECT_RATE);
    update_roof_effect_rate = false;
  }
}