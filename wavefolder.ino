#include "wavefolder.h"

void Wavefolder::Init() {
    SetDrive(0.0f);
    SetOffset(0.0f);
}

inline void Wavefolder::SetDrive(float gain) { 
  gain_ = 10.0f * gain + 1.0f;
  compens_ = fast_shape(0.09f - 3.05f * gain) * 0.77f + 1.0f ;
}

float Wavefolder::Process(float in) {
    float ft;
    float sgn;
    in += offset_;
    in *= gain_;
    ft  = floorf((in + 1.0f) * 0.5f);
    sgn = (static_cast<int>(ft) % 2 == 0) ? 1.0f : -1.0f;
    //int rem = static_cast<int>(ft) % 2 ;
    //sgn = (float)(1 - 2 * rem); // should work a bit faster ???
    return sgn * (in - 2.0f * ft) ; //* compens_;
}
