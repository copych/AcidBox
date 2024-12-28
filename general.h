#ifndef GENERAL_H
#define GENERAL_H
#include "config.h"

class General {
  public:
    static float fclamp(float in, float min, float max) __attribute__((noinline));
    static float fast_shape(float x) __attribute__((noinline));
    static void fast_sincos(float x, float* sinRes, float* cosRes) __attribute__((noinline));
    static float one_div(float a) __attribute__((always_inline));
    static void mixer() __attribute__((noinline));

    // TODO was not optimized in config.h with IRAM or other attributes
    static float dB2amp(float dB);
    static float amp2dB(float amp);
    static float linToExp(float in, float inMin, float inMax, float outMin, float outMax);
    static float knobMap(float in, float outMin, float outMax);

    // TODO, Not used
    // static float fast_sin(float x);
    // static float fast_cos(float x);
    // static float linToLin(float in, float inMin, float inMax, float outMin, float outMax);
    // static float expToLin(float in, float inMin, float inMax, float outMin, float outMax);
};

#endif