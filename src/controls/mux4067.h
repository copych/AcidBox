#pragma once

#include <Arduino.h>

/**
* Basic header only class to work with 4067 multiplexers with the selector pins connected in parallel.
* It supports 1 to 4 multiplexers (16 - 48 muxed lines), but can be easily extended.
*
* 1. In your program create an instance, providing 4 pin numbers for the address bus (S0-S3), and data pins (SIG) for each multiplexer:
*
*       Mux4067stack Mux(4,5,6,7, 15); // if you have 1 multiplexer
*           or
*       Mux4067stack Mux(4,5,6,7, 15,16); // if you have 2 multiplexers
*           etc..
* 
* 2. There's a typedef for a 2-dimensional array for GPIO polling, wrapped into a struct, so you just use it like this:
* 
*       Mux4067stack::READINGS* myReadings ;
* 
* 3. if you use muxer for buttons and encoders, you can get a pointer to an individual element like this
* 
*       uint8_t* myPointer = &(myReadings->Y[MUXER_ID][Y_PIN_NUMBER]);
*
* MIT License
* Copyright (c) 2024 copych 
*/


/** 
* In case we want performance increase on digitalRead() / digitalWrite(),
* we can use this library by Alex Gyver: https://github-com.translate.goog/GyverLibs/GyverIO?_x_tr_sl=ru&_x_tr_tl=en
* It is available in the Arduino IDE library manager, search for "GiverIO"
*/

// #define USE_GYVER_IO 
#ifdef USE_GYVER_IO
  #include "GyverIO.h"
  #define WRITE_FUNC    gio::write
  #define READ_FUNC     gio::read  
#else
  #define WRITE_FUNC    digitalWrite
  #define READ_FUNC     digitalRead
#endif

#if ACTIVE_STATE == LOW
  #define SIG_INPUT_MODE    INPUT_PULLUP  
#else
  #define SIG_INPUT_MODE    INPUT_PULLDOWN  
#endif


class Mux4067stack {
  public:
    // it is wrapped with a struct to ensure that memory mapping is always the same inside and outside this class
    // so you can "safely" provide a pointer to an individual element
    typedef struct {  uint8_t Y[NUM_MUXERS][16]; } READINGS;
    
    Mux4067stack() {;}
#if (NUM_MUXERS == 1)
    Mux4067stack(int s0, int s1, int s2, int s3, int sig0) {
      this->setPins(s0, s1, s2, s3, sig0);
    }
#elif (NUM_MUXERS == 2)
    Mux4067stack(int s0, int s1, int s2, int s3, int sig0, int sig1) {
      this->setPins(s0, s1, s2, s3, sig0, sig1);
    }
#elif (NUM_MUXERS == 3)
    Mux4067stack(int s0, int s1, int s2, int s3, int sig0, int sig1, int sig2) {
      this->setPins(s0, s1, s2, s3, sig0, sig1, sig2);
    }
#elif (NUM_MUXERS == 4)
    Mux4067stack(int s0, int s1, int s2, int s3, int sig0, int sig1, int sig2, int sig3) {
      this->setPins(s0, s1, s2, s3, sig0, sig1, sig2, sig3);
    }
#endif
    ~Mux4067stack() {;}
    
#if NUM_MUXERS == 1
    inline void setPins(int gpioS0, int gpioS1, int gpioS2, int gpioS3, int sig0) 
#elif NUM_MUXERS == 2
    inline void setPins(int gpioS0, int gpioS1, int gpioS2, int gpioS3, int sig0, int sig1) 
#elif NUM_MUXERS == 3
    inline void setPins(int gpioS0, int gpioS1, int gpioS2, int gpioS3, int sig0, int sig1, int sig2) 
#elif NUM_MUXERS == 4
    inline void setPins(int gpioS0, int gpioS1, int gpioS2, int gpioS3, int sig0, int sig1, int sig2, int sig3) 
#endif
    {
      pinS[0] = gpioS0;
      pinS[1] = gpioS1;
      pinS[2] = gpioS2;
      pinS[3] = gpioS3; 
      pinSig[0] = sig0;
#if NUM_MUXERS > 1
      pinSig[1] = sig1;
#endif
#if NUM_MUXERS > 2
      pinSig[2] = sig2;
#endif
#if NUM_MUXERS > 3
      pinSig[3] = sig3;
#endif
      setPinModes();
    };
    
    inline void reset() {
      connectByBits(0, 0, 0, 0);
    };
    
    inline void setPinModes(){
      // address pins
      for (int s=0; s<4; s++) { 
#ifdef USE_GYVER_IO
        gio::init(pinS[s]);
        gio::mode(pinS[s], OUTPUT);
#else        
        pinMode(pinS[s], OUTPUT);
#endif
      }
      // signal pins
      for (int i=0; i<NUM_MUXERS; i++) { 
#ifdef USE_GYVER_IO
        gio::init(pinSig[i]);
#endif
        pinMode(pinSig[i], SIG_INPUT_MODE); 
      }
    }
    
    // read all signal pins for the set address
    inline void read() {
      for (int i = 0 ; i < NUM_MUXERS; i++) {
        readings.Y[i][curAddress] = READ_FUNC(pinSig[i]);
      }
    };
    
    inline void readAll() {
      // it's not a mandatory to start with the line 0000, you'll get all the 16 lines anyways
      for (int i = 0; i < 16 ; i++) {
        
      //  Serial.print(curAddress);
      //  Serial.printf(" : bits : %d %d %d %d : read()\r\n", curAddressBits[0], curAddressBits[1], curAddressBits[2], curAddressBits[3]);
        // read currently connected mux lines
        read();
        connectNextLine();
        delayMicroseconds(1);// delay to avoid overloading the MUX
      }
      //Serial.println();
    };


    inline READINGS* getReadingsPointer() {
      /*
      Serial.println("getReadingsPointer ");
      for(int i = 0; i < NUM_MUXERS; i++) {
        for(int j = 0; j < 16; j++) {
          Serial.printf("%#010x ", &(readings.Y[i][j]));
        }
        Serial.println();
      }
      */
      return &readings;
    };
    
  protected:  
    uint8_t pinS[4];
    uint8_t pinSig[NUM_MUXERS];
    READINGS readings;
    int curAddressBits[4] = { 0 };
    int curAddress = 0;
 
    inline void flipBitInAddress(int bitN) {
      curAddressBits[bitN] = (curAddressBits[bitN] == 0) ? 1 : 0;
      buildCurrentAddress();
    }

    inline void buildCurrentAddress() {
      curAddress = curAddressBits[0] + (curAddressBits[1]<<1) + (curAddressBits[2]<<2) + (curAddressBits[3]<<3);
    }

    inline void calcCurrentBits() {
      curAddressBits[0] = curAddress & 0b0001;
      curAddressBits[1] = (curAddress & 0b0010) > 0;
      curAddressBits[2] = (curAddress & 0b0100) > 0;
      curAddressBits[3] = (curAddress & 0b1000) > 0;      
    }

    inline void connectNextLine() {
      int the_bit = gray_bit[gray_rev[curAddress]]; // 
      flipBitInAddress(the_bit);
      WRITE_FUNC(pinS[the_bit], curAddressBits[the_bit]); 
      //  Serial.printf("connectNextLine: pin %d = %d\r\n" , pinS[the_bit] , curAddressBits[the_bit] );
    }
    
    inline void connectCurrentAddress() {
      calcCurrentBits();
      WRITE_FUNC(pinS[0], curAddressBits[0]) ;
      WRITE_FUNC(pinS[1], curAddressBits[1]) ;
      WRITE_FUNC(pinS[2], curAddressBits[2]) ;
      WRITE_FUNC(pinS[3], curAddressBits[3]) ;
    };
    
    inline void connectCurrentBits() {
      buildCurrentAddress();
      WRITE_FUNC(pinS[0], curAddressBits[0]) ;
      WRITE_FUNC(pinS[1], curAddressBits[1]) ;
      WRITE_FUNC(pinS[2], curAddressBits[2]) ;
      WRITE_FUNC(pinS[3], curAddressBits[3]) ;
    };
  
    inline void connectByBits(int a0, int a1, int a2, int a3) {
      curAddressBits[0] = a0;
      curAddressBits[1] = a1;
      curAddressBits[2] = a2;
      curAddressBits[3] = a3;
      buildCurrentAddress();
      connectCurrentAddress();
    };
    
    inline void connectByAddress(uint8_t address) {
      curAddress = address;
      calcCurrentBits();
      connectCurrentAddress();
    };
    
  private:
    
    // Gray code here is used to minimize line selection time, cause we only flip one bit at a time
    const uint8_t gray_code[16] = { // 0 1 3 2 6 7 5 4 12 13 15 14 10 11 9 8
      0b0000,      0b0001,      0b0011,      0b0010,      0b0110,      0b0111,      0b0101,      0b0100,
      0b1100,      0b1101,      0b1111,      0b1110,      0b1010,      0b1011,      0b1001,      0b1000
    };
    
    // bitmask to flip, according to Gray code
    const uint8_t gray_mask[16] = { // 1 8 1 2 1 4 1 2 1 8 1 2 1 4 1 2
      0b0001,      0b0010,      0b0001,      0b0100,      0b0001,      0b0010,      0b0001,      0b1000,
      0b0001,      0b0010,      0b0001,      0b0100,      0b0001,      0b0010,      0b0001,      0b1000, 
    };
    
    // reverse lookup id by Gray code
    const uint8_t gray_rev[16] = {
      0,   1,  3,  2, 7, 6,  4,  5, 
      15, 14, 12, 13, 8, 9, 11, 10
    };
    
    // bit position to flip to get from current to next
    const uint8_t gray_bit[16] = {
      0, 1, 0, 2, 0, 1, 0, 3, 
      0, 1, 0, 2, 0, 1, 0, 3
    };
};
