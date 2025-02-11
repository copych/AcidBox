#pragma once

#include <Arduino.h>

#include "looper_config.h"
#include "../../config.h"
#include "pattern.h"
#include <vector>
#include <stdexcept>

namespace performer {

typedef enum {  TRACK_POLY,           // ordinary MIDI instrument
                TRACK_MONO,           // overlapped notes gives portamento
                TRACK_DRUMS,          // note lengths may not be recognized 
                NUM_TRACK_TYPES 
} eTrackType_t;

typedef enum {  LOOP_NONE,
                LOOP_PATTERN,
                LOOP_ALL,
                NUM_LOOP_MODES
} eLoopMode_t;

typedef struct
{
  int note = 0;
  int length = -1;
} noteStack;

class Track {
public:
  Track();
  Track(eTrackType_t track_type, byte midi_channel);

  eTrackType_t        getTrackType()    {return _trackType;};
  eLoopMode_t         getLoopMode()     {return _loopMode;};
  byte                getMidiChannel()  {return _midiChannel;};
  byte                getPrevNote()     {return _prevNote;};
  int                 getLength()       {return _length;};
  bool                isMute()          {return _mute;};
  bool                isSolo()          {return _solo;};
  
  Pattern* getPattern(int patternIndex);
  std::vector<Pattern>* getPatterns()   {return &Patterns;}
  void  setTrackType(eTrackType_t new_type)   {_trackType = new_type;};
  void  setLoopMode(eLoopMode_t new_mode)     {_loopMode = new_mode;};
  void  setMidiChannel(byte new_channel)      {_midiChannel = new_channel;};
  void  setPrevNote(byte val)                 {_prevNote = val;};
  void  setMuteOnOff(bool val)                {_mute = val;};
  void  setSoloOnOff(bool val)                {_solo = val;};
  int   addPattern(int length);
  bool  addStackNote(int note, int length);
  noteStack	    _noteStack[NOTE_STACK_SIZE];

private:
  std::vector<Pattern> Patterns;
  byte          _midiChannel      = 0;
  byte          _prevNote         = 0;
  eTrackType_t  _trackType        = TRACK_MONO;
  eLoopMode_t   _loopMode         = LOOP_PATTERN;
  int           _length           = 1;
  bool          _mute             = false;
  bool          _solo             = false;
};
} // namespace

