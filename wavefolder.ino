#include "wavefolder.h"

void Wavefolder::Init() {
    gain_   = 1.0f;
    offset_ = 0.0f;
}

float Wavefolder::Process(float in) {
    float ft, sgn;
    in += offset_;
    in *= gain_;
    ft  = floorf((in + 1.0f) * 0.5f);
    // sgn = static_cast<int>(ft) % 2 == 0 ? 1.0f : -1.0f;
    sgn = (float)((int)1 - (int)2 * (static_cast<int>(ft) % 2)); // should work a bit faster ???
    return sgn * (in - 2.0f * ft);
}
