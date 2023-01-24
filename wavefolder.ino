#include "wavefolder.h"

void Wavefolder::Init() {
    gain_   = 1.0f;
    offset_ = 0.0f;
}

float Wavefolder::Process(float in) {
    float ft, sgn;
    int rem;
    in += offset_;
    in *= gain_;
    ft  = floorf((in + 1.0f) * 0.5f);
    sgn = static_cast<int>(ft) % 2 == 0 ? 1.0f : -1.0f;
    //rem = static_cast<int>(ft) % 2 ;
    //sgn = (float)(1 - 2 * rem); // should work a bit faster ???
    return sgn * (in - 2.0f * ft);
}
