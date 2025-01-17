#pragma once

#include <Arduino.h>

/**
* Simple class for handling rotary encoders
* 
* 1. Create an instance:
*     MuxEncoder encoder;
*
* 2. Submit encoder id, pointers to the GPIO readings, your callback function and your encoder type e.g.:
*     uint8_t A, B; 
*     myEncHandler(int id, int dir) {
*       // id is what you submit via bind()
*       // dir is -1 for counter-clock-wise or 1 for clock-wise rotation
*       ... 
*     }
*     encoder.bind(0, &A, &B, myEncHandler, MuxEncoder::MODE_HALF_STEP);
* 
* 3. Poll process() in a loop:
*     while(i_want_enc_processing) {
*       A = digitalRead(PIN_A);
*       B = digitalRead(PIN_B);
*       encoder.process();
*     }
*
* MIT License
* Copyright (c) 2024 copych 
*/


class MuxEncoder {
 
  public:
     MuxEncoder() {;}
    ~MuxEncoder() {;}
  
    // encoder types: how many pulses per click do you have?
    enum encMode        { MODE_HALF_STEP, MODE_FULL_STEP, MODE_COUNT };

    static constexpr int8_t stepIncrement[MODE_COUNT][16] = { // the idea was taken from Alex Gyver's examples
      {0, 1, -1, 0,  -1, 0, 0, 1,  1, 0, 0, -1,  0, -1, 1, 0}, // Half-step encoder
      {0, 0,  0, 0,  -1, 0, 0, 1,  1, 0, 0, -1,  0,  0, 0, 0}  // Full-step encoder
    };
    
    inline void bind(uint8_t id, uint8_t* a, uint8_t* b, std::function<void(int, int)> cb, encMode mode = MODE_FULL_STEP) {
      _a = a;
      _b = b;
      _mode = mode;
      _id = id;
      _callbackFunc = cb;
/*
      Serial.print("enc bind ");
      Serial.print(id);
      Serial.print(" ");
      Serial.printf("%#010x %#010x\r\n", _a, _b);
*/
    }
  
    void process() {
      unsigned int clk = ( *_a == ACTIVE_STATE);
      unsigned int dt = (*_b == ACTIVE_STATE);
/*
      Serial.print("enc process ");
      Serial.print(_id);
      Serial.print(" ");
      Serial.print(clk);
      Serial.print(" ");
      Serial.println(dt);
*/

      newState = (clk | dt << 1);
      if (newState != oldState) {
        int stateMux = newState | (oldState << 2);
        int rotation = stepIncrement[_mode][stateMux];
        oldState = newState;
        if (rotation != 0) {
          _callbackFunc(_id , rotation);
        }
      }
    }

  protected:
    int newState = 0;
    int oldState = 0;
  
  private:
    uint8_t _id;
    uint8_t* _a = nullptr;
    uint8_t* _b = nullptr;
    encMode _mode = MODE_FULL_STEP;
    std::function<void(int, int)> _callbackFunc;
};
