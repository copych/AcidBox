
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

// reciprocal asm injection for xtensa LX6 FPU
float General::one_div(float a) {
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

// void IRAM_ATTR General::mixer() { // sum buffers 
// #ifdef DEBUG_MASTER_OUT
//   static float meter = 0.0f;
// #endif
//   static float synth1_out_l, synth1_out_r, synth2_out_l, synth2_out_r, drums_out_l, drums_out_r;
//   static float dly_l, dly_r, rvb_l, rvb_r;
//   static float mono_mix;
//     dly_k1 = Synth1._sendDelay;
//     dly_k2 = Synth2._sendDelay;
//     dly_k3 = Drums._sendDelay;
// #ifndef NO_PSRAM 
//     rvb_k1 = Synth1._sendReverb;
//     rvb_k2 = Synth2._sendReverb;
//     rvb_k3 = Drums._sendReverb;
// #endif
//     for (int i=0; i < DMA_BUF_LEN; i++) { 
//       drums_out_l = Drums.drums_buf_l[current_out_buf][i];
//       drums_out_r = Drums.drums_buf_r[current_out_buf][i];

//       synth1_out_l = Synth1.GetPan() * Synth1.synth_buf[current_out_buf][i];
//       synth1_out_r = (1.0f - Synth1.GetPan()) * Synth1.synth_buf[current_out_buf][i];
//       synth2_out_l = Synth2.GetPan() * Synth2.synth_buf[current_out_buf][i];
//       synth2_out_r = (1.0f - Synth2.GetPan()) * Synth2.synth_buf[current_out_buf][i];

      
//       dly_l = dly_k1 * synth1_out_l + dly_k2 * synth2_out_l + dly_k3 * drums_out_l; // delay bus
//       dly_r = dly_k1 * synth1_out_r + dly_k2 * synth2_out_r + dly_k3 * drums_out_r;
//       Delay.Process( &dly_l, &dly_r );
// #ifndef NO_PSRAM
//       rvb_l = rvb_k1 * synth1_out_l + rvb_k2 * synth2_out_l + rvb_k3 * drums_out_l; // reverb bus
//       rvb_r = rvb_k1 * synth1_out_r + rvb_k2 * synth2_out_r + rvb_k3 * drums_out_r;
//       Reverb.Process( &rvb_l, &rvb_r );

//       mix_buf_l[current_out_buf][i] = (synth1_out_l + synth2_out_l + drums_out_l + dly_l + rvb_l);
//       mix_buf_r[current_out_buf][i] = (synth1_out_r + synth2_out_r + drums_out_r + dly_r + rvb_r);
// #else
//       mix_buf_l[current_out_buf][i] = (synth1_out_l + synth2_out_l + drums_out_l + dly_l);
//       mix_buf_r[current_out_buf][i] = (synth1_out_r + synth2_out_r + drums_out_r + dly_r);
// #endif
//       mono_mix = 0.5f * (mix_buf_l[current_out_buf][i] + mix_buf_r[current_out_buf][i]);
//   //    Comp.Process(mono_mix);     // calculate gain based on a mono mix

//       Comp.Process(drums_out_l*0.25f);  // calc compressor gain, side-chain driven by drums


//       mix_buf_l[current_out_buf][i] = (Comp.Apply( 0.25f * mix_buf_l[current_out_buf][i]));
//       mix_buf_r[current_out_buf][i] = (Comp.Apply( 0.25f * mix_buf_r[current_out_buf][i]));

      
// #ifdef DEBUG_MASTER_OUT
//       if ( i % 16 == 0) meter = meter * 0.95f + fabs( mono_mix); 
// #endif
//   //    mix_buf_l[current_out_buf][i] = General::fclamp(mix_buf_l[current_out_buf][i] , -1.0f, 1.0f); // clipper
//   //    mix_buf_r[current_out_buf][i] = General::fclamp(mix_buf_r[current_out_buf][i] , -1.0f, 1.0f);
//      mix_buf_l[current_out_buf][i] = General::fast_shape( mix_buf_l[current_out_buf][i]); // soft limitter/saturator
//      mix_buf_r[current_out_buf][i] = General::fast_shape( mix_buf_r[current_out_buf][i]);
//    }
// #ifdef DEBUG_MASTER_OUT
//   meter *= 0.95f;
//   meter += fabs(mono_mix); 
//   DEBF("out= %0.5f\r\n", meter);
// #endif
// }


// TODO, not used
// float IRAM_ATTR General::fast_sin(float x) { // 8.798 MOP/S full period lookup table. With table size = 32+1, max error is about 0.5% 
//   float f, res;
//   int i;
//   int sign = x < 0.0;
//   x = sign ? -x : x;
//   float index = (float)x * NORM_RADIANS  ;
//   i = CYCLE_INDEX((int)index);
//   f = ((float)index - (int)index);
//   res = f * (Tables::sin_tbl[i+1] - Tables::sin_tbl[i]) + Tables::sin_tbl[i];
//   return  sign ? -res : res;
// }

// TODO, not used
// inline float IRAM_ATTR General::fast_cos(float x) { // 7.666 MOP/S full period lookup table. With table size = 32+1, max error is about 0.5% 
//   float f, res, index;
//   int i;
//   x += PI_DIV_TWO;
//   int sign = x < 0.0;
//   x = sign ? -x : x;
//   index = x * NORM_RADIANS ;
//   i = CYCLE_INDEX((int)index);
//   f = (index - (int)index);
//   res = f * (Tables::sin_tbl[i+1] - Tables::sin_tbl[i]) + Tables::sin_tbl[i];
//   return  sign ? -res : res;
// }

// float General::linToLin(float in, float inMin, float inMax, float outMin, float outMax) {
//   // map input to the range 0.0...1.0:
//   float tmp = (in-inMin) * General::one_div(inMax-inMin);

//   // map the tmp-value to the range outMin...outMax:
//   tmp *= (outMax-outMin);
//   tmp += outMin;

//   return tmp;
// }

// float General::expToLin(float in, float inMin, float inMax, float outMin, float outMax){
//   float tmp = logf(in * General::one_div(inMin)) * General::one_div( logf(inMax * General::one_div(inMin)));
//   return outMin + tmp * (outMax-outMin);
// }