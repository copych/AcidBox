#ifndef GENERAL_H
#define GENERAL_H
#include "config.h"

#ifndef acidbox_max
#define acidbox_max(a, b) ((a < b) ? b : a)
#endif

#ifndef acidbox_min
#define acidbox_min(a, b) ((a < b) ? a : b)
#endif

class General {
  public:
    static float fclamp(float in, float min, float max) __attribute__((noinline));
    static float fast_shape(float x) __attribute__((noinline));
    static void fast_sincos(float x, float* sinRes, float* cosRes) __attribute__((noinline));
    
    // Included implementation in header to make inlining possible
    static float one_div(float a) __attribute__((always_inline)) {
      // reciprocal asm injection for xtensa LX6 FPU
      float result;
      asm volatile (
          "wfr f1, %1"          "\n\t"
          "recip0.s f0, f1"     "\n\t"
          "const.s f2, 1"       "\n\t"
          "msub.s f2, f1, f0"   "\n\t"
          "maddn.s f0, f0, f2"  "\n\t"
          "const.s f2, 1"       "\n\t"
          "msub.s f2, f1, f0"   "\n\t"
          "maddn.s f0, f0, f2"  "\n\t"
          "rfr %0, f0"          "\n\t"
          : "=r" (result)
          : "r" (a)
          : "f0","f1","f2"
      );
      return result;
    }

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