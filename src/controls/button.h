#pragma once

#include <Arduino.h>
/**
* MuxButton is a class that handles buttons
*
* 1. Create an instance per button connected:
*     MuxButton myButton;
* 
* 2. Declare the stuff
*     uint8_t SW;
*     myBtnHandler(int id, MuxButton::btnEvents evt) {
*       // id is what you submit via bind()
*       // evt is one of MuxButton::EVENT_TOUCH, MuxButton::EVENT_PRESS, MuxButton::EVENT_LONGPRESS,
*       //               MuxButton::EVENT_AUTOCLICK, MuxButton::EVENT_RELEASE, MuxButton::EVENT_CLICK
*       ...
*     }
*
* 3. During setup() submit button id, the pointer to the GPIO readings and your callback function e.g.:
*     myButton.bind(0, &SW, myBtnHandler);
*
* 4.Poll process() in a loop:
*     while(i_want_btn_processing) {
*        SW = digitalRead(PIN_SW);
*        myButton.process();
*     }
*
* MIT License
* Copyright (c) 2024 copych 
*/

class MuxButton {
  public:
    MuxButton() {;}
    ~MuxButton() {;}
    
    enum btnEvents        { EVENT_TOUCH, EVENT_PRESS, EVENT_LONGPRESS, EVENT_AUTOCLICK, EVENT_RELEASE, EVENT_CLICK, EVENTS_COUNT };
    
    inline void bind(uint8_t id, uint8_t* input, std::function<void(int, btnEvents)> cb) {
      _input = input;
      _id = id;
      _callbackFunc = cb;
/*
      Serial.print("btn bind ");
      Serial.print(id);
      Serial.print(" ");
      Serial.printf("%#010x\r\n", _input,);
*/
    }
    
    inline void process() {
      buttonState = (*_input == ACTIVE_STATE);
      size_t ms = millis();
        if ( buttonState != oldButtonState) {
          // it changed
          if ( buttonState == 1 ) {                  // rise (the beginning)
            // launch rise timer to debounce
            if (buttonActive == 1) {                                // it's active, but rises again
              // it might be noise, we check the state and timers
              if (pressActive == 1) {
                // it has passed the rise check, so it may be the ending phase
                // or it may be some noise during pressing
              } else {
                // we are still checking the rise for purity
                if (ms - riseTimer > riseThreshold) {
                  // not a good place to confirm but we have to
                  pressActive = 1;
                }
              }
            } else {
              // it wasn't active lately, now it's time
              buttonActive = 1;                 // fun begins for this button
              riseTimer = ms;
              longPressTimer = ms;              // workaround for random longPresses
              autoFireTimer = ms;               // workaround              
              _callbackFunc(_id, EVENT_TOUCH);        // I am the first          
            }
          } else {                                                // fall (is it a click or just an end?)
            // launch fall timer to debounce
            fallTimer = ms;
            // pendingClick = 1;
          }
        } else {                                                  // no change for this button
          if ( buttonState == 1 ) {                               // the button reading is "active"
            // someone's pushing our button
            if ( pressActive == 0 && (ms - riseTimer > riseThreshold)) {
              pressActive = 1;
              longPressTimer = ms;
              _callbackFunc(_id, EVENT_PRESS);
            }
            if (pressActive == 1 && longPressActive == 0 && (ms - longPressTimer > longPressThreshold)) {
              longPressActive = 1;
              _callbackFunc(_id, EVENT_LONGPRESS);
            }
            if (autoFireEnabled == 1 && longPressActive == 1) {
              if (ms - autoFireDelay > autoFireTimer) { 
                autoFireTimer = ms;
                _callbackFunc(_id, EVENT_AUTOCLICK); 
              }
            }
          } else {                                                // the button reading is "inactive"
            // ouch! a click?
            if (buttonActive == 1 && (ms - fallTimer > fallThreshold)) {
              // yes, this is a click
              buttonActive = 0; // bye-bye
              pressActive = 0;
              if (longPressActive == 0 || lateClickEnabled == 1) {
                _callbackFunc(_id, EVENT_CLICK);
              }
              longPressActive = 0;
              _callbackFunc(_id, EVENT_RELEASE);
            }
          }
        }
      oldButtonState = buttonState;
    }

    inline void setAutoClick(boolean onOff) {
      autoFireEnabled = onOff ? 1 : 0;
    }

    inline void  enableLateClick(boolean onOff) {
      lateClickEnabled = onOff ? 1 : 0;
    }

    inline void setRiseTimeMs(uint32_t ms) {
      riseThreshold = constrain(ms, 0 , 100); // default 20
    }
    
    inline void setFallTimeMs(uint32_t ms) {
      fallThreshold = constrain(ms, 0 , 100); // default 10
    }
    
    inline void setLongPressDelayMs(uint32_t ms) {
      longPressThreshold = constrain(ms, 0 , 4000); // default 800
    }

    inline void setAutoFirePeriodMs(uint32_t ms) {
      autoFireDelay = constrain(ms, 0 , 4000); // default 500
    }
    
    inline bool pressed() {
      return buttonActive;
    }
    
  protected:
    uint32_t longPressThreshold = 800;            // the threshold (in milliseconds) before a long press is detected 
    uint32_t autoFireDelay      = 500;            // the threshold (in milliseconds) between clicks if autofire is enabled 
    uint32_t riseThreshold      = 20;             // the threshold (in milliseconds) for a button press to be confirmed (i.e. debounce, not "noise") 
    uint32_t fallThreshold      = 10;             // debounce, not "noise", also this is the time, while new "touches" won't be registered
    uint32_t buttonState        = 0;              // current readings              
    uint32_t oldButtonState     = 0;              // previous readings              
    uint32_t activeButtons      = 0;              // activity bitmask              
    uint32_t currentMillis;                       // not to call repeatedly within loop
    uint32_t riseTimer;                           // stores the time that the button was pressed (relative to boot time)
    uint32_t fallTimer;                           // stores the duration (in milliseconds) that the button was pressed/held down for
    uint32_t longPressTimer;                      // stores the duration (in milliseconds) that the button was pressed/held down for
    uint32_t autoFireTimer;                       // milliseconds before firing autoclick
    uint32_t autoFireEnabled    = 0;              // should the buttons generate continious clicks when pressed longer than a longPressThreshold
    uint32_t lateClickEnabled   = 0;              // enable registering click after a longPress call
    uint32_t buttonActive;                        // it's 1 since first touch till the click is confirmed
    uint32_t pressActive;                         // indicates if the button has been pressed and debounced
    uint32_t longPressActive;                     // indicates if the button has been long-pressed
  
  private:
    uint32_t _id;
    uint8_t* _input = nullptr;    
    std::function<void(int, btnEvents)> _callbackFunc;
};
