#include "pattern.h"
#include "misc.h"
#include <math.h>

using namespace performer;

void Pattern::generateDrums(eStyle_t style, float intensity /*0.0 - 1.0*/, float tension /*0.0 - 1.0*/){
  for (int i = 0; i < MAX_PATTERN_STEPS; i++) {
    Steps[i].clear();
  }
  generateDrumInstr(DRUM_BD, style, intensity, tension);
  generateDrumInstr(DRUM_SD, style, intensity, tension);
  generateDrumInstr(DRUM_OH, style, intensity, tension);
  generateDrumInstr(DRUM_CH, style, intensity, tension);
  generateDrumInstr(DRUM_PE, style, intensity, tension);
 // generateDrumInstr(DRUM_CR, style, intensity*0.1f, tension);
  xorDrumParts(DRUM_CH, DRUM_OH, 100, DRUM_NORMAL);
}


void Pattern::xorDrumParts(eDrumInstr_t instrToFill, eDrumInstr_t BaseInstr, int chance, byte velo) {
  for (int i = 0; i < MAX_PATTERN_STEPS; i++) {
    if (!checkEvent(i, EVT_NOTE_ON, BaseInstr)) {
      if (flip(chance)) {
        addEvent(i, EVT_NOTE_ON, _drum_notes[instrToFill], velo); 
      }      
    }
  }
}

void Pattern::generateDrumInstr(eDrumInstr_t instr, eStyle_t style, float intensity /*0.0 - 1.0*/, float randomness /*0.0 - 1.0*/){
  int8_t rnd_vel;     
  float note_chance;
  for (int i = 0; i < MAX_PATTERN_STEPS; i++) {
    switch(instr) {
      case DRUM_BD:
        note_chance = _BD_chances[style][i % 16];
        break;
      case DRUM_SD:
        note_chance = _SD_chances[style][i % 16];
        break;
      case DRUM_OH:
        note_chance = _OH_chances[style][i % 16];
        break;
      case DRUM_CH:
        note_chance = _CH_chances[style][i % 16];
        break;
      case DRUM_PE:
        note_chance = _CH_chances[style][i % 16];
        break;
      case DRUM_CR:
        note_chance = _BD_chances[style][i % 16];
        break;
      case DRUM_CY:
        note_chance = _CH_chances[style][i % 16];
        break;
      default:
        note_chance = 50;
        break;
    }
    
    if (flip((int)(intensity * note_chance))) {
      if (flip((int)(60 * intensity))) rnd_vel = DRUM_ACCENTED; else rnd_vel = DRUM_NORMAL;
      addEvent(i, EVT_NOTE_ON, _drum_notes[instr], rnd_vel);
    }

    
  }
}

void Pattern::generateMelody(byte root_note /*0-127*/, eStyle_t style, float intensity /*0.0 - 1.0*/, float randomness /*0.0 - 1.0*/, float tension /*0.0 - 1.0*/){
  int8_t rnd_vel;
  int8_t rnd_note;
  int8_t cur_note = -1;
  for (int i = 0; i < MAX_PATTERN_STEPS; i++) {
    Steps[i].clear();
    int note_chance = (float)_note_chances[i % 16];
    if (flip(intensity * note_chance)) {
      rnd_note = _current_note_set[(random(_current_note_set.size()))] + root_note;
      if (flip((int)(60 * intensity))) rnd_vel = VEL_ACCENTED; else rnd_vel = VEL_NORMAL;
      addEvent(i, EVT_NOTE_ON, rnd_note, rnd_vel);
      cur_note = rnd_note;
      if (flip((int)(70 * intensity))) {
        addEvent(i, EVT_CONTROL_CHANGE, CC_PORTAMENTO, MIDI_VAL_ON);
      } else {      
        addEvent(i, EVT_CONTROL_CHANGE, CC_PORTAMENTO, MIDI_VAL_OFF);
      }
    } else {
      if (cur_note != -1) {
        if (flip(50)) {
          addEvent(i, EVT_NOTE_OFF, cur_note, 0);
          cur_note = -1;
        }
      }
    }
  }
}

void Pattern::generateNoteSet(float tension /*0.0 - 1.0*/, float randomness){
    _current_note_set.clear();
    int idx = round(tension * _semitones.size());
    _current_note_set = _semitones[idx];
}

bool Pattern::isSlide(int step_num) {
  if (checkEvent(step_num, EVT_CONTROL_CHANGE, CC_PORTAMENTO, MIDI_VAL_ON) ) return true; else return false; 
}



String Pattern::toText() {
  String outStr = "";
  uint8_t patt[12][MAX_PATTERN_STEPS];
  for ( int i = 0 ; i < _length; i++ ) {
    for (int j = 0 ; j < 12; j++ ) {
      patt[j][i] = 0 ;
      for ( auto &st: Steps[i]) {
        if (st.type == EVT_NOTE_ON && st.value1 == j ) {
          if (st.value2 <= DRUM_MIDDLE) {
            patt[j][i] = 1; 
          } else {
            patt[j][i] = 2;
          }
        }       
      }
    }
  }
  for (int j = 0; j < 12; j++) {
    outStr += (String)_drum_names[j] + ":";
    for (int i = 0; i < _length; i++) {
      if ( patt[j][i] == 1 ) {
        outStr += "x";
      } else if ( patt[j][i] == 2 ) {
        outStr += "X";
      } else {
        outStr += ".";
      }
    }
    outStr += '\r';
    outStr += '\n';
  }
  return outStr;
}

void Pattern::addEvent(int step_num, eEventType_t evt_type, byte val1, byte val2) {
  Steps[step_num].emplace_back(evt_type, val1, val2);
}

bool Pattern::checkEvent(int step_num, eEventType_t evt_type) {
  bool res = false;
  for ( auto &st: Steps[step_num] ) {
    if (st.type == evt_type) {
      res = true;
      break;
    }
  }
  return res;
}

bool Pattern::checkEvent(int step_num, eEventType_t evt_type, byte val1) {
  bool res = false;
  for ( auto &st: Steps[step_num] ) {
    if (st.type == evt_type && st.value1 == val1) {
      res = true;
      break;
    }
  }
  return res;
}

bool Pattern::checkEvent(int step_num, eEventType_t evt_type, byte val1, byte val2) {
  bool res = false;
  for ( auto &st: Steps[step_num] ) {
    if (st.type == evt_type && st.value1 == val1 && st.value2 == val2) { 
      res = true;
      break;
    }
  }
  return res;
}
