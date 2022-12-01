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


#include <stdint.h>
#include "FastLED.h"

#define DEFAULT_TRANSITION_TIME 500
#define DEFAULT_BRIGHTNESS      255
#define DEFAULT_LED_STATE       true
#define DEFAULT_COLOR1          CRGB::Red
#define DEFAULT_COLOR2          CRGB::Green
#define DEFAULT_STYLE           XMasComms::SIMPLE
#define DEFAULT_EFFECT_RATE     20

#ifndef __XMAS_COMMS_H__
#define __XMAS_COMMS_H__

namespace XMasComms {

  const int SLAVE_I2C_ADDR = 42;

  // LED Styles
  const char* LED_STYLES = "off;simple;twinkle;fade_between";

  enum LEDStyles : uint8_t {
    OFF          = 0,
    SIMPLE       = 1,
    TWINKLE      = 2,
    FADE_BETWEEN = 3
  };

  // Field Definitions
  enum XMasControllerField : uint8_t {
    TRANSITION_TIME    = 0,
    GARAGE_COLOR1      = 1,
    GARAGE_COLOR2      = 2,
    GARAGE_STYLE       = 3,
    GARAGE_EFFECT_RATE = 4,
    DOOR_COLOR1        = 5,
    DOOR_COLOR2        = 6,
    DOOR_STYLE         = 7,
    DOOR_EFFECT_RATE   = 8,
    ROOF_COLOR1        = 9,
    ROOF_COLOR2        = 10,
    ROOF_STYLE         = 11,
    ROOF_EFFECT_RATE   = 12
  };

  struct TransitionTime {
    unsigned short time;
  };

  struct Color {
    CRGB    rgb;
    uint8_t brightness;
    bool    state;
  };

  struct Style {
    LEDStyles style;
  };

  struct EffectRate {
    unsigned short rate;
  };

  // Packet formats
  union TransitionTimePacket {
    TransitionTime data;
    uint8_t        buf[sizeof(TransitionTime)];
  };

  union ColorPacket {
    Color   data;
    uint8_t buf[sizeof(Color)];
  };

  union StylePacket {
    Style   data;
    uint8_t buf[sizeof(Style)];
  };

  union EffectRatePacket {
    EffectRate data;
    uint8_t    buf[sizeof(EffectRate)];
  };
};

#endif


