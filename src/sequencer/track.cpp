#include "track.h"


using namespace performer;

int   Track::addPattern() {
  Patterns.emplace_back(Pattern());
  return (Patterns.size()-1);
}
