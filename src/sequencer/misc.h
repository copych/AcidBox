#pragma once
#include <Arduino.h>
#include "../../config.h"

inline bool flip(int percent) {
  if (random(101) <= percent) return true; else return false;
}

inline void MidiInit() {  
#ifdef MIDI_VIA_SERIAL
  Serial.begin(115200);
#endif
#ifdef MIDI_VIA_SERIAL2
  pinMode( MIDIRX_PIN , INPUT_PULLDOWN);
  pinMode( MIDITX_PIN , OUTPUT);
  Serial2.begin( 31250, SERIAL_8N1, MIDIRX_PIN, MIDITX_PIN ); // midi port
#endif
}


#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))