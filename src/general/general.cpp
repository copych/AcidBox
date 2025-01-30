
#include "tables.h"
#include "general.h"

float IRAM_ATTR General::fclamp(float in, float min, float max) {
  if (in>max) return max;
  if (in<min) return min;
  return in;
}

float IRAM_ATTR General::fast_shape(float x) {
    int sign = 1;
    if (x<0) {
      x = -x;
      sign = -1;
    }
   
    if (x>=4.95f) {
      return (float)sign; // tanh(x) ~= 1, when |x| > 4
    }

  //  if (x<=0.4f) return float(x*sign) * 0.9498724f; // smooth region borders; tanh(x) ~= x, when |x| < 0.4 
    float res = Tables::lookupTable(Tables::shaper_tbl, (x*SHAPER_LOOKUP_COEF)); // lookup table contains tanh(x), 0 <= x <= 5
    return sign<0 ? -res : res;
  // float poly = (2.12f-2.88f*x+4.0f*x*x);
   // return sign * x * (poly * one_div(poly * x + 1.0f)); // very good approximation found here https://www.musicdsp.org/en/latest/Other/178-reasonably-accurate-fastish-tanh-approximation.html
                                                    // but it uses float division which is not that fast on esp32
}


void IRAM_ATTR General::fast_sincos(float x, float* sinRes, float* cosRes) {
  float xc, f, res, index;
  int i, sign;
  sign = x < 0.0;
  xc = x;
  x = sign ? -x : x;
  index = x * NORM_RADIANS  ;
  i = CYCLE_INDEX((int)index);
  f = ((float)index - (int)index);
  res = f * (Tables::sin_tbl[i+1] - Tables::sin_tbl[i]) + Tables::sin_tbl[i];
  *sinRes =  sign ? -res : res;

  x = xc + PI_DIV_TWO;
  sign = x < 0.0;
  x = sign ? -x : x;
  index = x * NORM_RADIANS ;
  i = CYCLE_INDEX((int)index);
  res = f * (Tables::sin_tbl[i+1] - Tables::sin_tbl[i]) + Tables::sin_tbl[i];
  *cosRes = sign ? -res : res;
}

float General::dB2amp(float dB){
  return expf(dB * 0.11512925464970228420089957273422f);
  //return pow(10.0, (0.05*dB)); // naive, inefficient version
}

float General::amp2dB(float amp){
  return 8.6858896380650365530225783783321f * logf(amp);
  //return 20*log10(amp); // naive version
}

float General::linToExp(float in, float inMin, float inMax, float outMin, float outMax){
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) * General::one_div(inMax-inMin);

  // map the tmp-value exponentially to the range outMin...outMax:
  //tmp = outMin * exp( tmp*(log(outMax)-log(outMin)) );
  return outMin * expf( tmp*(logf(outMax * General::one_div(outMin))) );
}

float General::knobMap(float in, float outMin, float outMax) {
  return outMin + Tables::lookupTable(Tables::knob_tbl, (int)(in * TABLE_SIZE)) * (outMax - outMin);
}