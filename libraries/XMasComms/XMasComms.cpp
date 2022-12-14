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

#include "XMasComms.h"


void print_packet(XMasPacket *pkt) {
  XMAS_DEBUG_PRINTLN(F("=== XMasPacket ==="))

  XMAS_DEBUG_PRINT(F("Size: "))
  XMAS_DEBUG_PRINTLN(sizeof(XMasPacket))
  XMAS_DEBUG_PRINTLN(F(""))


  XMAS_DEBUG_PRINTLN(F("Color 1: "))
  XMAS_DEBUG_PRINT(F("\tRed: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.color1.red)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("\tGreen: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.color1.green)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("\tBlue: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.color1.blue)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINTLN(F("Color 2: "))
  XMAS_DEBUG_PRINT(F("\tRed: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.color2.red)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("\tGreen: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.color2.green)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("\tBlue: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.color2.blue)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("Effect Rate: "))
  XMAS_DEBUG_PRINT(pkt->data.effect_rate)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("Transition Time: "))
  XMAS_DEBUG_PRINT(pkt->data.transition_time)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINT(F("Animation: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->data.animation)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINTLN(F(""))

  XMAS_DEBUG_PRINT(F("CRC: "))
  XMAS_DEBUG_PRINT_HEX((uint8_t)pkt->crc)
  XMAS_DEBUG_PRINTLN(F(""))
  XMAS_DEBUG_PRINTLN(F("=== End XMasPacket ==="))
}

// Calculate the 8-bit CRC of the input data using the specified polynomial
crc_t xmas_crc8(const uint8_t* data, size_t length) {
  // Initialize the CRC value to 0
  crc_t crc = 0;

  // Loop over each byte in the input data
  for (size_t i = 0; i < length; i++) {
    // XOR the next byte of the data with the current value of the CRC
    crc ^= data[i];

    // Perform the modulo-2 division for each bit in the byte
    for (int j = 0; j < 8 * sizeof(crc_t); j++) {
      // If the most significant bit of the CRC is set, XOR the CRC with the polynomial
      if (crc & 0b10000000) {
        crc = (crc << 1) ^ crc_poly;
      }
      // Otherwise, just shift the CRC left by one bit
      else {
        crc <<= 1;
      }
    }
  }

  // Return the final CRC value
  return crc;
}


bool send_packet(RHGenericDriver &driver, XMasPacket *pkt) {
  XMAS_DEBUG_PRINT(F("Sending Packet"))
  print_packet(pkt);

  pkt->crc = xmas_crc8((uint8_t*)&pkt->data, sizeof(pkt->data));

  if (!driver.send((uint8_t*)pkt, sizeof(XMasPacket))) {
    XMAS_DEBUG_PRINTLN(F("Send failed..."))
    return false;
  }

  if (!driver.waitPacketSent()) {
    XMAS_DEBUG_PRINTLN(F("Wait Packet Send Failed..."))
    return false;
  }

  XMAS_DEBUG_PRINTLN(F("Send Succeeded!"))
  return true;
}


bool recv_packet(RHGenericDriver &driver, XMasPacket *pkt) {
  crc_t crc;
  uint8_t len = sizeof(XMasPacket);

  XMAS_DEBUG_PRINTLN(F("Receiving Packet"))

  if (!driver.recv((uint8_t*)pkt, &len)) {
    XMAS_DEBUG_PRINTLN(F("Receive failed..."))
    return false;
  }

  XMAS_DEBUG_PRINTLN(F("Message Received"))
  print_packet(pkt);

  if (pkt->crc != xmas_crc8((uint8_t*)&pkt->data, sizeof(pkt->data))) {
    XMAS_DEBUG_PRINTLN(F("CRC of recieved packet does not match..."))
    return false;
  }

  return true;
}
