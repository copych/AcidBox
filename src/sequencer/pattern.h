#pragma once

#include <Arduino.h>
#include "looper_config.h"

namespace performer {

typedef enum  { DRUM_BD,        // Bass Drum
                DRUM_SD,        // Snare Drum
                DRUM_LT,        // Low Tom
                DRUM_MT,        // Mid Tom
                DRUM_HT,        // High Tom
                DRUM_CH,        // Closed Hat
                DRUM_OH,        // Open Hat
                DRUM_CY,        // ride CYmbal
                DRUM_CR,        // CRash cymbal
                DRUM_PE,        // PErcussion
                NUM_DRUM_INSTRS                
} eDrumInstr_t;

typedef enum  { STYLE_STRAIGHT,
                STYLE_TECHNOPOP,
                STYLE_BIGBEAT,
                STYLE_BREAK,
                STYLE_HANG,
                STYLE_TEST_LOAD,
                SLIDE_TEST_LOAD,
                NUM_STYLES
} eStyle_t;


typedef enum  { EVT_NONE,
                EVT_NOTE_ON,
                EVT_NOTE_OFF,
                EVT_PAUSE,          // unlike EVT_NONE, EVT_PAUSE can have set value1 meaning note number used for slide
                EVT_PITCHBEND,
                EVT_MODWHEEL,
                EVT_CONTROL_CHANGE,
                EVT_SYSEX,
                NUM_EVT_TYPES
} eEventType_t;   

struct sStepEvent_t{
                eEventType_t type = EVT_NONE;
                byte value1 = 0;
                byte value2 = 0;
                int length = NOTE_LENGTH_303;
                //construct
                sStepEvent_t (eEventType_t evt_type, byte val1, byte val2) :    type(evt_type), value1(val1), value2(val2)    {}
} ;

class Pattern {
  public:

  static constexpr char *str_events[NUM_EVT_TYPES] = {"EVT_NONE", "EVT_NOTE_ON", "EVT_NOTE_OFF", "EVT_PAUSE", "EVT_PITCHBEND", "EVT_CONTROL_CHANGE", "EVT_SYSEX"};

  // eDrumInstr_t                               BD    SD    LT    MT    HT    CH    OH    CY    CR    PE                 
  static  constexpr int8_t _drum_notes[NUM_DRUM_INSTRS] = { 0,    1,    2,    3,    3,    6,    7,    8,    9,    4 };

  //  MIDI notes seq:                           bd0   sd1   lt2   ht3   rm4   cp5   ch6   oh7   cy8   cr9   co10   cl11
  static constexpr char *_drum_names[12] =               { "BD", "SD", "LT", "HT", "RM", "CP", "CH", "OH", "CY", "CR", "CO", "CK"};

  static constexpr uint8_t _BD_chances[NUM_STYLES][16] = {
    {100, 0, 0, 0, 100,  0, 0, 25,  100, 0, 30,  0, 100,  0, 25,  25},                // STYLE_STRAIGHT,
    {100, 50,  25,  50,  0, 25,  25,  100, 50,  50,  90, 25,  0, 25,  0, 25},         // STYLE_TECHNOPOP,
    {100, 0, 0, 0, 0, 0, 25,  25,  0, 0, 0, 0, 0, 0, 0, 0},                           // STYLE_BIGBEAT,
    {100, 0, 0, 0, 75,  0, 0, 0, 90, 25,  50,  25,  90, 25,  50,  25},                // STYLE_BREAK,
    {100, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 50, 0},                              // STYLE_HANG,
    {100, 0, 0, 0, 100, 0, 0, 0, 100, 0, 0, 0, 100, 0, 0, 0}                          // STYLE_TEST_LOAD
  };

  static constexpr uint8_t _SD_chances[NUM_STYLES][16] = {
    {0, 0, 0, 0, 100, 0, 0, 25,  0, 0, 0, 0, 100, 50,  50,  25},                      // STYLE_STRAIGHT,
    {0, 0, 0, 0, 100, 0, 0, 25,  0, 0, 0, 0, 100, 50,  50,  25},                      // STYLE_TECHNOPOP,
    {0, 0, 0, 0, 75,  0, 0, 75,  0, 50,  0, 0, 100, 50,  75,  100},                   // STYLE_BIGBEAT,
    {0, 0, 0, 0, 100, 0, 0, 25,  0, 0, 0, 0, 100, 50,  50,  25},                      // STYLE_BREAK,
    {0, 0, 0, 0, 100, 0, 0, 25,  0, 0, 0, 0, 100, 50,  50,  25},                      // STYLE_HANG,
    {0, 0, 0, 0, 100, 0, 0, 0, 0, 0, 0, 0, 100, 0, 0, 100}                            // STYLE_TEST_LOAD
  };

  static constexpr uint8_t _CH_chances[NUM_STYLES][16] = {
    {100, 100, 25, 50,  100, 100, 25, 50,  100, 100, 25, 50,  100, 100, 25, 100},     // STYLE_STRAIGHT,
    {100, 100, 100, 75, 100, 75, 100, 100, 100, 100, 75,  100, 100, 100, 100, 100},   // STYLE_TECHNOPOP,
    {100, 100, 0, 50,  100, 100, 0, 50,  100, 100, 0, 50,  100, 100, 0, 100},         // STYLE_BIGBEAT,
    {100, 100, 0, 50,  100, 100, 0, 50,  100, 100, 0, 50,  100, 100, 0, 100},         // STYLE_BREAK,
    {100, 100, 0, 50,  100, 100, 0, 50,  100, 100, 0, 50,  100, 100, 0, 100},         // STYLE_HANG,
    {100, 100, 0, 100, 100, 100, 0, 100, 100, 100, 0, 100, 100, 100, 0, 100}          // STYLE_TEST_LOAD
  };

  static constexpr uint8_t _OH_chances[NUM_STYLES][16] = {
    {0, 0, 50,  0,  0, 0, 50,  0,  0, 0, 50,  0, 0, 0, 50,  0},                       // STYLE_STRAIGHT,
    {0, 0, 50,  0,  0, 0, 50,  0,  0, 0, 50,  0, 0, 0, 50,  0},                       // STYLE_TECHNOPOP,
    {0, 0, 50,  0,  0, 0, 50,  0,  0, 0, 50,  0, 0, 0, 50,  0},                       // STYLE_BIGBEAT,
    {0, 0, 50,  0,  0, 0, 50,  0,  0, 0, 50,  0, 0, 0, 50,  0},                       // STYLE_BREAK,
    {0, 0, 50,  0,  0, 0, 50,  0,  0, 0, 50,  0, 0, 0, 50,  0},                       // STYLE_HANG,
    {0, 0, 100, 0,  0, 0, 100, 0,  0, 0, 100, 0, 0, 0, 100, 0}                        // STYLE_TEST_LOAD
  };

  // TODO, fix below vectors because each pattern instance will contain this vector with vectors 
  const std::vector< std::vector<int8_t> > _semitones = {
    {0},                                        // just root note
    {0, 0, 0, 12, 24},                          // tonic + octaves
    {0, 0, 7, 14, 24, 24},                      // 5th, add 9th
    {0, 0, 12, 14, 15, 19},                     // add 10th, 11th
    {0, 0, 12, 24, 27},                         // minor 3rd 2 octaves higher
    {0, 0, 0, 7, 12, 15, 17, 20, 24},           // minor + 5th + octaved 3rd, 4th, 5th, 6th 
    {0, 0, 0, 12, 10, 19, 26, 27},              // 7th, 2nd and minor 3rd 2 octaves higher
    {0, 0, 0, 1, 7, 10, 12, 13},                // frigian mode + 5th + 7th
    {0, 0, 0, 0, 0, 0, 1, 13, 25},              // frigian mode
    {0, 0, 0, 12, 12, 13, 16, 19, 22, 24, 25},  // frigian mode, but maj3rd + 7th + 5th
    {0, 0, 12, 12, 18, 24, 24}                  // locrian mode? dim 5th
  };  

  Pattern() {};
  int             getLength()     {return _length;};
  bool            isActive()      {return _active;};
  bool            checkEvent(int step_num, eEventType_t evt_type);
  bool            checkEvent(int step_num, eEventType_t evt_type, byte val1);
  bool            checkEvent(int step_num, eEventType_t evt_type, byte val1, byte val2);
  String          toText();
  
  void addEvent(int step_num, eEventType_t evt_type, byte val1, byte val2); 
  void addEvent(int step_num, sStepEvent_t event);
  void setLength(int new_length)          {_length = constrain(new_length, 1, MAX_PATTERN_STEPS);};
  void setActiveOnOff(bool val)           {_active = val;}; 
  void generateDrums(eStyle_t style, float intencity /*0.0 - 1.0*/, float tension /*0.0 - 1.0*/);
  void generateDrumInstr(eDrumInstr_t instr, eStyle_t style, float intencity /*0.0 - 1.0*/, float randomness /*0.0 - 1.0*/);
  void generateMelody(byte root_note /*0-127*/, eStyle_t style, float intencity /*0.0 - 1.0*/, float randomness /*0.0 - 1.0*/, float tension /*0.0 - 1.0*/);
  void generateNoteSet(float tension /*0.0 - 1.0*/, float randomness = 0.0);
  void xorDrumParts(eDrumInstr_t instrToFill, eDrumInstr_t BaseInstr, int chance, byte velo); 
  std::vector<sStepEvent_t> Steps[MAX_PATTERN_STEPS]; 
  
private:
  int               _noteset    = 0;
  float             _intensity  = 0.5;
  float             _tension    = 0.5;
  int8_t            _root_note  = 60;
  int               _length     = MAX_PATTERN_STEPS;   // 16th notes
  bool              _active     = true;
  int               _id         = -1;
  int8_t            _note_chances[16]   = {77, 63, 63, 37, 63, 63, 50, 63, 37, 77, 90, 10, 77, 63, 57, 57};
  int8_t            _slide_chances[16]  = {10, 90, 50, 90, 50, 40, 60, 60, 40, 90, 50, 90, 30, 30, 30, 60};
  int8_t            _accent_chances[16] = {77, 63, 63, 37, 63, 63, 50, 63, 37, 77, 90, 10, 77, 63, 57, 57};
  
  std::vector<int8_t> _current_note_set = {_root_note};
};
} // namespace
