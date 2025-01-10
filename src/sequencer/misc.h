#pragma once
#include <Arduino.h>



inline float fclamp(float in, float minV, float maxV){
  if (in>maxV) return maxV;
  if (in<minV) return minV;
  return in;
}

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

// serial debugging macros

#ifndef DEBUG
  #ifdef DEBUG_ON
    #define DEB(...) Serial.print(__VA_ARGS__) 
    #define DEBF(...) Serial.printf(__VA_ARGS__) 
    #define DEBUG(...) Serial.println(__VA_ARGS__) 
  #else
    #define DEB(...)
    #define DEBF(...)
    #define DEBUG(...)
  #endif
#endif
