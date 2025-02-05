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
  switch (style) {
    case SLIDE_TEST_LOAD:
      clearPattern();
      for (int i = 0; i < MAX_PATTERN_STEPS; i++) {
        Steps[i].clear();
        int8_t c = root_note;
        int8_t plain = 60;
        int8_t accent = 100;
        switch (i)
        {
        // Check cases here: https://www.antonsavov.net/articles/303andmidi/#_how_does_the_midi_implementation_work_in_a_303clone
        // case 1          
        case 0:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c, accent);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c, true, true);
          break;
        }
        case 1:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+2, accent);
          // addEvent(i, event);
          // Set notes
          setNote(i, c+2, true, false);
          break;
        }
        // case 2      
        case 2:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+2, plain);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c+2, false, true);
          break;
        }
        case 3:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+2, plain);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c+2, false, true);
          break;
        }
        case 4:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+1, accent);
          // addEvent(i, event);
          // Set notes
          setNote(i, c+1, true, false);
          break;
        }
        // case 3
        case 5:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c, plain);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c, false, true);
          break;
        }
        case 6:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c, accent);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c, false, true);
          break;
        }
        case 7:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c, accent);
          // addEvent(i, event);
          // Set notes
          setNote(i, c, true, false);
          break;
        }
        // case 4
        case 8:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c, plain);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c, false, true);
          break;
        }
        case 9:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c, accent);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c, true, true);
          break;
        }
        case 10:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+3, plain);
          // addEvent(i, event);
          // Set notes
          setNote(i, c+3, false, false);
          break;
        }        
        // case 5
        case 11:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+4, plain);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c+4, false, true);
          break;
        }
        case 12:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+4, accent);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c+4, true, true);
          break;
        }
        case 13:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+4, plain);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c+4, false, true);
          break;
        }  
        case 14:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+4, accent);
          // event.length = event.length + SLIDE_LENGTH_303;
          // addEvent(i, event);
          // Set notes
          setNote(i, c+4, true, true);
          break;
        }  
        case 15:
        {
          // sStepEvent_t event(EVT_NOTE_ON, c+4, plain);
          // addEvent(i, event);
          // Set notes
          setNote(i, c+4, false, false);
          break;
        }          
        default:
          break;
        }
      }
      break;      
    case STYLE_TEST_LOAD:
      for (int i = 0; i < MAX_PATTERN_STEPS; i++) {
        Steps[i].clear();
        if (i == 7 || i == 15) {
          cur_note = root_note+13;
          rnd_vel = 60;
          addEvent(i, EVT_CONTROL_CHANGE, CC_PORTAMENTO, MIDI_VAL_ON);
        } else {
          cur_note = root_note;
          //addEvent(i, EVT_NOTE_OFF, cur_note, 0);
          if (i==15) rnd_vel = 100;
          addEvent(i, EVT_CONTROL_CHANGE, CC_PORTAMENTO, MIDI_VAL_OFF);
        }
        addEvent(i, EVT_NOTE_ON, cur_note, rnd_vel);
        //addEvent(i, EVT_NOTE_OFF, cur_note, 0);
      }
      break;
    default:
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
}

void Pattern::generateNoteSet(float tension /*0.0 - 1.0*/, float randomness){
    _current_note_set.clear();
    int idx = round(tension * _semitones.size());
    _current_note_set = _semitones[idx];
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

void Pattern::setNote(unsigned int step, byte note, bool accent, bool slide) {
  if(step > _length -1) {
    throw std::invalid_argument("Step for note out of bounds");
  } else if(note > 127) {
    throw std::invalid_argument("Note ranges from 0 to 127");
  }
  Notes[step].type = EVT_NOTE_ON;
  Notes[step].value1 = note;
  if(slide) {
    Notes[step].length = NOTE_LENGTH_303 + SLIDE_LENGTH_303;
  }
  if(!accent) {
    Notes[step].value2 = VEL_NORMAL;
  } else {
    Notes[step].value2 = VEL_ACCENTED;
  }
}

sStepEvent_t Pattern::getNote(int step) {
  if(step > _length -1) {
    throw std::invalid_argument("Step for note out of bounds");
  }
  return Notes[step];
}

void Pattern::clearNote(unsigned int step) {
  if(step > _length -1) {
    throw std::invalid_argument("Step for note out of bounds");
  }
  Notes[step].type = EVT_NONE;
  Notes[step].value1 = 0;
  Notes[step].value2 = 0;
  Notes[step].length = 0;
}

void Pattern::clearPattern() {
  for (int i = 0; i < _length; i++) {
    clearNote(i);
  }
}


void Pattern::addEvent(int step_num, eEventType_t evt_type, byte val1, byte val2) {
  Steps[step_num].emplace_back(evt_type, val1, val2);
}

void Pattern::addEvent(int step_num, sStepEvent_t event) {
  Steps[step_num].emplace_back(event);
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
  for (auto &st : Steps[step_num]) {
    if (st.type == evt_type && st.value1 == val1 && st.value2 == val2) {
      res = true;
      break;
    }
  }
  return res;
}
