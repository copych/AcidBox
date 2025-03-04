/* 
 This file is the copy of the part of MIDIUSB Library for Arduino
 https://github.com/arduino-libraries/MIDIUSB/blob/master/src/MIDIUSB_Defs.h
 Licensing statement is here
 https://github.com/arduino-libraries/MIDIUSB/blob/master/LICENSE.txt
*/

#pragma once
#include <stdint.h>

typedef struct
{
	uint8_t header;
	uint8_t byte1;
	uint8_t byte2;
	uint8_t byte3;
} midiEventPacket_t;