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
#include <stdlib.h>

#include <RHGenericDriver.h>

#include "FastLED.h"

#include "SmartLED.h"

/////////////////////////////////////////////////////////////
// Debug Setup
/////////////////////////////////////////////////////////////

#define SERIAL_DEBUG

#ifdef SERIAL_DEBUG
  #define XMAS_DEBUG_PRINT(x) Serial.print(x);
  #define XMAS_DEBUG_PRINT_HEX(x) {Serial.print("0x"); Serial.print(x, HEX);}
  #define XMAS_DEBUG_PRINTLN(x) Serial.println(x);
#else
  #define XMAS_DEBUG_PRINT(x)
  #define XMAS_DEBUG_PRINT_HEX(x)
  #define XMAS_DEBUG_PRINTLN(x)
#endif

///////////////////////////////////////////////////
// Definitions
///////////////////////////////////////////////////
#define DEFAULT_LORA_FREQ        ((float) 915.0)
#define DEFAULT_LORA_CAD_TIMEOUT 10000

#define MAX_PACKET_DATA 100

#define DEFAULT_TRANSITION_TIME 500
#define DEFAULT_BRIGHTNESS      255
#define DEFAULT_LED_STATE       true
#define DEFAULT_COLOR1          CRGB::Red
#define DEFAULT_COLOR2          CRGB::Green
#define DEFAULT_STYLE           SIMPLE
#define DEFAULT_EFFECT_RATE     20

///////////////////////////////////////////////////
// Header File
///////////////////////////////////////////////////

#ifndef __XMAS_COMMS_H__
#define __XMAS_COMMS_H__

///////////////////////////////////////////////////
// Types
///////////////////////////////////////////////////

typedef uint8_t crc_t;

struct __attribute__((packed)) XMasData {
  CRGB color1;
  CRGB color2;
  short effect_rate;
  short transition_time;
  AnimationType animation;
};

struct __attribute__((packed)) XMasPacket {
  XMasData data;
  crc_t crc;
};

typedef XMasData LedInfo;

///////////////////////////////////////////////////
// Helper Functions
///////////////////////////////////////////////////

void print_packet(XMasPacket *pkt);

// Packet CRC Calculation
const uint8_t crc_poly = 0b100011011;
crc_t xmas_crc8(const uint8_t* data, size_t length);

// Send packet
bool send_packet(RHGenericDriver &driver, XMasPacket *pkt);

// Recieve Packet
bool recv_packet(RHGenericDriver &driver, XMasPacket *pkt);

#endif