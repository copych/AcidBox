
inline float bilinearLookup(float (&table)[16][16], float x, float y) {
  float const kmap = 0.1181f; // map from 0-127 to 0-14.99
  int32_t i,j;
  float fi,fj;
  float v1,v2,v3,v4;
  float res1,res2,res3;
  x *= kmap;
  y *= kmap;
  i = (int32_t)x;
  j = (int32_t)y;
  fi = (float)x - i;
  fj = (float)y - j;
  v1 = table[i][j];
  v2 = table[i+1][j];
  v3 = table[i][j+1];
  v4 = table[i+1][j+1];  
  res1 = (float)fi * (float)(v2-v1) + v1;
  res2 = (float)fi * (float)(v4-v3) + v3;
  res3 = (float)fj * (float)(res2-res1) + res1;
  return res3;
}

inline float  lookupTable(float (&table)[TABLE_SIZE+1], float index ) { // lookup value in a table by float index, using linear interpolation
  float v1, v2, res;
  int32_t i;
  float f;
 // if (index >= TABLE_SIZE) return table[TABLE_SIZE];
  i = (int32_t)index;
  f = (float)index - i;
  v1 = (table)[i];
  v2 = (table)[i+1];
  res = (float)f * (float)(v2-v1) + v1;
 // DEBF("i %0.6f mantissa %0.6f v1 %0.6f v2 %0.6f \r\n" , index , f , v1, v2  );
  return res;
}

inline float IRAM_ATTR fclamp(float in, float min, float max){
    if (in>max) return max;
    if (in<min) return min;
    return in;
}

inline float IRAM_ATTR fast_shape(float x){
    int sign = 1;
    if (x<0) {
      x = -x;
      sign = -1;
    }
   
    if (x>=4.95f) {
      return (float)sign; // tanh(x) ~= 1, when |x| > 4
    }

  //  if (x<=0.4f) return float(x*sign) * 0.9498724f; // smooth region borders; tanh(x) ~= x, when |x| < 0.4 
    float res = lookupTable(shaper_tbl, (x*SHAPER_LOOKUP_COEF)); // lookup table contains tanh(x), 0 <= x <= 5
    return sign<0 ? -res : res;
  // float poly = (2.12f-2.88f*x+4.0f*x*x);
   // return sign * x * (poly * one_div(poly * x + 1.0f)); // very good approximation found here https://www.musicdsp.org/en/latest/Other/178-reasonably-accurate-fastish-tanh-approximation.html
                                                    // but it uses float division which is not that fast on esp32
}


void IRAM_ATTR fast_sincos(float x, float* sinRes, float* cosRes){
  float xc, f, res, index;
  int i, sign;
  sign = x < 0.0;
  xc = x;
  x = sign ? -x : x;
  index = x * NORM_RADIANS  ;
  i = CYCLE_INDEX((int)index);
  f = ((float)index - (int)index);
  res = f * (sin_tbl[i+1] - sin_tbl[i]) + sin_tbl[i];
  *sinRes =  sign ? -res : res;

  x = xc + PI_DIV_TWO;
  sign = x < 0.0;
  x = sign ? -x : x;
  index = x * NORM_RADIANS ;
  i = CYCLE_INDEX((int)index);
  res = f * (sin_tbl[i+1] - sin_tbl[i]) + sin_tbl[i];
  *cosRes = sign ? -res : res;
}

float IRAM_ATTR fast_sin(float x) { // 8.798 MOP/S full period lookup table. With table size = 32+1, max error is about 0.5% 
  float f, res;
  int i;
  int sign = x < 0.0;
  x = sign ? -x : x;
  float index = (float)x * NORM_RADIANS  ;
  i = CYCLE_INDEX((int)index);
  f = ((float)index - (int)index);
  res = f * (sin_tbl[i+1] - sin_tbl[i]) + sin_tbl[i];
  return  sign ? -res : res;
}

inline float IRAM_ATTR fast_cos(float x) { // 7.666 MOP/S full period lookup table. With table size = 32+1, max error is about 0.5% 
  float f, res, index;
  int i;
  x += PI_DIV_TWO;
  int sign = x < 0.0;
  x = sign ? -x : x;
  index = x * NORM_RADIANS ;
  i = CYCLE_INDEX((int)index);
  f = (index - (int)index);
  res = f * (sin_tbl[i+1] - sin_tbl[i]) + sin_tbl[i];
  return  sign ? -res : res;
}


// reciprocal asm injection for xtensa LX6 FPU
static inline float __attribute__((always_inline)) one_div(float a) {
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

float dB2amp(float dB){
  return expf(dB * 0.11512925464970228420089957273422f);
  //return pow(10.0, (0.05*dB)); // naive, inefficient version
}

float amp2dB(float amp){
  return 8.6858896380650365530225783783321f * logf(amp);
  //return 20*log10(amp); // naive version
}

float linToLin(float in, float inMin, float inMax, float outMin, float outMax){
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) * one_div(inMax-inMin);

  // map the tmp-value to the range outMin...outMax:
  tmp *= (outMax-outMin);
  tmp += outMin;

  return tmp;
}

inline float linToExp(float in, float inMin, float inMax, float outMin, float outMax){
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) * one_div(inMax-inMin);

  // map the tmp-value exponentially to the range outMin...outMax:
  //tmp = outMin * exp( tmp*(log(outMax)-log(outMin)) );
  return outMin * expf( tmp*(logf(outMax * one_div(outMin))) );
}



float expToLin(float in, float inMin, float inMax, float outMin, float outMax){
  float tmp = logf(in * one_div(inMin)) * one_div( logf(inMax * one_div(inMin)));
  return outMin + tmp * (outMax-outMin);
}

float knobMap(float in, float outMin, float outMax) {
  return outMin + lookupTable(knob_tbl, (int)(in * TABLE_SIZE)) * (outMax - outMin);
}
