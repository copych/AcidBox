#include "track.h"

using namespace performer;

Track::Track() {

};

Track::Track(eTrackType_t track_type, byte midi_channel) {
  _trackType = track_type; 
  _midiChannel = midi_channel;
};

int Track::addPattern() {
  Patterns.emplace_back(Pattern());
  return (Patterns.size()-1);
}

bool Track::addStackNote(int note, int length)
{
  for(unsigned char i = 0; i < NOTE_STACK_SIZE; i++) {
    if (_noteStack[i].length == -1) {
      _noteStack[i].length = length;  
      _noteStack[i].note = note;
      return true;
    }
  }
  return false;
}
