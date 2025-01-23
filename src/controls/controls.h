#pragma once
#include <Arduino.h>

#define NUM_MUXERS    3     // can be 1 .. 4
#define ACTIVE_STATE  LOW   // LOW = switch connects to GND, HIGH = switch connects to 3V3

#define MUX_S0  15
#define MUX_S1  16
#define MUX_S2  17
#define MUX_S3  18
#define MUX0_SIG  11
#define MUX1_SIG  12
#define MUX2_SIG  13

typedef enum  {ENC_MENU, ENC_CUTOFF, ENC_RESO, ENC_ENVMOD, ENC_DECAY, ENC_ACCENT, ENC_DRIVE, NUM_ENCODERS} eEncoders;
typedef enum  {BTN_PLAY, BTN_STOP, BTN_MENU,
              BTN_MUTE1, BTN_MUTE2, BTN_MUTE3, 
              BTN_SELECT1, BTN_SELECT2, BTN_SELECT3, 
              BTN_LEFT, BTN_UP, BTN_RIGHT, BTN_DOWN,
              BTN_SEQ1, BTN_SEQ2, BTN_SEQ3, BTN_SEQ4, BTN_SEQ5, BTN_SEQ6, BTN_SEQ7, BTN_SEQ8, 
              BTN_SEQ9, BTN_SEQ10, BTN_SEQ11, BTN_SEQ12, BTN_SEQ13, BTN_SEQ14, BTN_SEQ15, BTN_SEQ16,
              BTN_FUNC1, BTN_FUNC2,
              NUM_BUTTONS} eButtons;
typedef enum {PRM_SYN1_VOL, PRM_SYN2_VOL, PRM_SMP_VOL,
              PRM_SYN1_CUTOFF, PRM_SYN2_CUTOFF, PRM_SMP_CUTOFF,
              PRM_SYN1_RESO, PRM_SYN2_RESO,
              PRM_SYN1_DECAY, PRM_SYN2_DECAY,
              PRM_SYN1_ENVMOD, PRM_SYN2_ENVMOD,
              PRM_SYN1_ACCENT, PRM_SYN2_ACCENT,
              PRM_SYN1_DRIVE, PRM_SYN2_DRIVE, PRM_SMP_CRUSHER,
              PRM_SYN1_TUNING, PRM_SYN2_TUNING,
              NUM_PARAMS} eParams;
typedef struct {
  eParams id;
  float minVal;
  float maxVal;
} param_t;
typedef struct {
  uint8_t mux_id; 
  uint8_t mux_Y;
} muxPin_t;


#include "src/controls/mux4067.h"
#include "src/controls/encoder.h"
#include "src/controls/button.h"

// it has static members, so only to be istantiated once
class UIControls {
  public:
    UIControls() {;};
    ~UIControls() {;};
    
    // encoders and buttons arrays
    MuxEncoder enc[NUM_ENCODERS];
    MuxButton  btn[NUM_BUTTONS];  
    
    // stacked multiplexers object
    Mux4067stack Mux;
    
    // callback function to handle buttons events, it receives the id of the fired button and the event kind
    static void onButton(int btn_id, MuxButton::btnEvents evt) ;

    // callback function which receives the id of the moved encoder, and the direction +/- 1
    static void onEncoder(int enc_id, int dir) ;
    
    void begin();
    
    // to call eternally in loop
    inline void polling();
  
  private:
    // 2-dimensional array for GPIO polling [NUM_MUXERS][16], wrapped into a struct
    Mux4067stack::READINGS* readings ;
    
    // arrays containing addresses of the encoders and buttons {mux_id, Y_pin)
    const muxPin_t encA_pins[NUM_ENCODERS] = {{0,0}, {0,1}, {0,2}, {0,3}, {0,4}, {0,5}, {0,6}};
    const muxPin_t encB_pins[NUM_ENCODERS] = {{1,0}, {1,1}, {1,2}, {1,3}, {1,4}, {1,5}, {1,6}};
    const muxPin_t btn_pins[NUM_BUTTONS] = {
    /* BTN_PLAY */    {0,7},
    /* BTN_STOP */    {0,8},
    /* BTN_MENU */    {0,9},
    /* BTN_MUTE1 */   {0,10},
    /* BTN_MUTE2 */   {0,11},
    /* BTN_MUTE3 */   {0,12},
    /* BTN_SELECT1 */ {0,13},
    /* BTN_SELECT2 */ {0,14},
    /* BTN_SELECT3 */ {0,15},
    /* BTN_LEFT */    {1,7},
    /* BTN_UP */      {1,8},
    /* BTN_RIGHT */   {1,9},
    /* BTN_DOWN */    {1,10},
    /* BTN_FUNC1 */   {1,11},
    /* BTN_FUNC2 */   {1,12},
    /* BTN_SEQ]1*/    {2,0},
    /* BTN_SEQ2 */    {2,1},
    /* BTN_SEQ3 */    {2,2},
    /* BTN_SEQ4 */    {2,3},
    /* BTN_SEQ5 */    {2,4},
    /* BTN_SEQ6 */    {2,5},
    /* BTN_SEQ7 */    {2,6},
    /* BTN_SEQ8 */    {2,7},
    /* BTN_SEQ9 */    {2,8},
    /* BTN_SEQ10 */   {2,9},
    /* BTN_SEQ11 */   {2,10},
    /* BTN_SEQ12 */   {2,11},
    /* BTN_SEQ13 */   {2,12},
    /* BTN_SEQ14 */   {2,13}, 
    /* BTN_SEQ15 */   {2,14},
    /* BTN_SEQ16 */   {2,15}
    };


    
};


  
inline void UIControls::polling() {
  // read all inputs into the readings array
  Mux.readAll();
  /*
  for (int i = 0; i < NUM_MUXERS; i++) {
    for (int j = 0; j < 16; j++) {
      Serial.print(readings->Y[i][j]);
      Serial.print(" ");
    }
    Serial.println();
  }
  Serial.println();
  delay(1000);
  */
  // process A and B for each encoder
  for (int i = 0 ; i < NUM_ENCODERS; i++) {
    enc[i].process();
  }
  // process every button
  for (int i = 0 ; i < NUM_BUTTONS; i++) {
    btn[i].process();
  }
}