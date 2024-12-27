#ifndef GENERAL_H
#define GENERAL_H

class General {
  public:
    static float fclamp(float in, float min, float max) __attribute__((noinline));
    static float fast_shape(float x) __attribute__((noinline));
    static void fast_sincos(float x, float* sinRes, float* cosRes) __attribute__((noinline));
    static float one_div(float a) __attribute__((always_inline));
    // TODO was not optimized in config.h
    static float dB2amp(float dB);
    
    // TODO was not optimized in config.h
    static float amp2dB(float amp);

    // TODO, Not used
    // static float fast_sin(float x);
    // static float fast_cos(float x);
};

#endif