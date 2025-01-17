#include "controls.h"

void UIControls::begin() {
  Mux.setPins(MUX_S0,  MUX_S1,  MUX_S2,  MUX_S3,  MUX0_SIG, MUX1_SIG, MUX2_SIG);
  // reset to zero
  Mux.reset();
  // direct access to polling results
  readings = Mux.getReadingsPointer(); 
  // setup encoders Encoder::bind(given_id, pointer_to_A, pointer_to_B, callback(int id, int direction), encoder_mode)
  for (int i = 0; i < NUM_ENCODERS; i++) {
    enc[i].bind(i, &(readings->Y[encA_pins[i].mux_id][encA_pins[i].mux_Y]), &(readings->Y[encB_pins[i].mux_id][encB_pins[i].mux_Y]), onEncoder, MuxEncoder::MODE_FULL_STEP);
  }
    // pins on the first muxer
  for (int i = 0; i < NUM_BUTTONS; i++) {
    btn[i].bind(i, &(readings->Y[btn_pins[i].mux_id][btn_pins[i].mux_Y]), onButton);
  }
}

// callback function to handle buttons events, it receives the id of the fired button and the event kind
void UIControls::onButton(int id, MuxButton::btnEvents evt) {
  Serial.print("button id ");
  Serial.print(id);
  Serial.print(" event ");
  Serial.print(evt);
  Serial.print(": ");
  switch(evt) {
    case MuxButton::EVENT_PRESS:
        Serial.println("PRESS");
        break;
    case MuxButton::EVENT_TOUCH:
        Serial.println("TOUCH");
        break;
    case MuxButton::EVENT_CLICK: 
        Serial.println("CLICK"); 
        break;
    case MuxButton::EVENT_RELEASE: 
        Serial.println("RELEASE"); 
        break;
    case MuxButton::EVENT_LONGPRESS: 
        Serial.println("LONG PRESS"); 
        break;
    case MuxButton::EVENT_AUTOCLICK: 
        Serial.println("AUTO-CLICK"); 
        break;
    default:
      {
        Serial.println("OTHER");
      }
  }
}

// callback function which receives the id of the moved encoder, and the direction +/- 1
void UIControls::onEncoder(int id, int dir) {
  Serial.print("encoder id ");
  Serial.print(id);
  Serial.print(" dir ");
  Serial.println(dir);
}
