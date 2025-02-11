#include "track.h"

using namespace performer;

Track::Track() {

};

Track::Track(eTrackType_t track_type, byte midi_channel) {
  _trackType = track_type; 
  _midiChannel = midi_channel;
};

int Track::addPattern(int length) {
  ePatternType_t pattern = (_trackType == TRACK_DRUMS) ? DRUM : SYNTH;
  Patterns.emplace_back(Pattern(length, pattern));
  return (Patterns.size()-1);
}

Pattern* Track::getPattern(int patternIndex) {
  if(Patterns.size() > patternIndex) {
    return &Patterns[patternIndex];
  }
  throw std::invalid_argument("Patterns vector out of bounds");
}

bool Track::addStackNote(int note, int length)
{
  DEBF("Add to stack note: %d, length: %d\r\n", note, length);
  for(unsigned char i = 0; i < NOTE_STACK_SIZE; i++) {
    if (_noteStack[i].length == -1) {
      _noteStack[i].length = length;  
      _noteStack[i].note = note;
      return true;
    }
  }
  return false;
}
