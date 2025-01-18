#include "track.h"

using namespace performer;

int   Track::addPattern() {
  Patterns.emplace_back(Pattern());
  return (Patterns.size()-1);
}

bool Track::addStackNote(int note, bool isSlide)
{
  for(unsigned char i = 0; i < NOTE_STACK_SIZE; i++) {
    if (_noteStack[i].length == -1) {
      int noteLength = NOTE_LENGTH_303;
      if(isSlide) {
        noteLength = noteLength + SLIDE_LENGTH_303;          
      }
      _noteStack[i].length = noteLength;  
      _noteStack[i].note = note;
      return true;
    }
  }
  return false;
}

