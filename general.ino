static void drums_generate() {
  // float fl_sample = 0.0f;
  // float fr_sample = 0.0f;
    for (int i=0; i < DMA_BUF_LEN; i++){
      Drums.Process( &drums_buf_l[i], &drums_buf_r[i] );      
    } 
}

static void mixer() { // sum buffers 
#ifdef DEBUG_MASTER_OUT
  static float meter = 0.0f;
#endif
  static float synth1_out_l, synth1_out_r, synth2_out_l, synth2_out_r, drums_out_l, drums_out_r;
  static float dly_l, dly_r, rvb_l, rvb_r;
  static float mono_mix;
    dly_k1 = Synth1._sendDelay;
    dly_k2 = Synth2._sendDelay;
    dly_k3 = Drums._sendDelay;
#ifndef NO_PSRAM 
    rvb_k1 = Synth1._sendReverb;
    rvb_k2 = Synth2._sendReverb;
    rvb_k3 = Drums._sendReverb;
#endif
    for (int i=0; i < DMA_BUF_LEN; i++) { 
      synth1_out_l = Synth1.GetPan()*synth_buf[0][i];
      synth1_out_r = (1.0f-Synth1.GetPan())*synth_buf[0][i];
      synth2_out_l = Synth2.GetPan()*synth_buf[1][i];
      synth2_out_r = (1.0f-Synth2.GetPan())*synth_buf[1][i];
      drums_out_l = drums_buf_l[i];
      drums_out_r = drums_buf_r[i];
      dly_l = dly_k1 * synth1_out_l + dly_k2 * synth2_out_l + dly_k3 * drums_out_l; // delay bus
      dly_r = dly_k1 * synth1_out_r + dly_k2 * synth2_out_r + dly_k3 * drums_out_r;
      Delay.Process( &dly_l, &dly_r );
#ifndef NO_PSRAM
      rvb_l = rvb_k1 * synth1_out_l + rvb_k2 * synth2_out_l + rvb_k3 * drums_out_l; // reverb bus
      rvb_r = rvb_k1 * synth1_out_r + rvb_k2 * synth2_out_r + rvb_k3 * drums_out_r;
      Reverb.Process( &rvb_l, &rvb_r );

      mix_buf_l[i] = 0.2f * (synth1_out_l + synth2_out_l + drums_out_l + dly_l + rvb_l);
      mix_buf_r[i] = 0.2f * (synth1_out_r + synth2_out_r + drums_out_r + dly_r + rvb_r);
#else
      mix_buf_l[i] = 0.25f * (synth1_out_l + synth2_out_l + drums_out_l + dly_l);
      mix_buf_r[i] = 0.25f * (synth1_out_r + synth2_out_r + drums_out_r + dly_r);
#endif
      mono_mix = 0.5*(mix_buf_l[i] + mix_buf_r[i]);
      Comp.Process(mono_mix); // calculate gain based on a mono mix, can be a side chain
      mix_buf_l[i] = (Comp.Apply(mix_buf_l[i]));
      mix_buf_r[i] = (Comp.Apply(mix_buf_r[i]));
      
#ifdef DEBUG_MASTER_OUT______________
      if ( i % 16 == 0) meter = meter * 0.95f + abs( mono_mix); 
#endif
   //   mix_buf_l[i] = fclamp(mix_buf_l[i] , -1.0f, 1.0f); // clipper
    //  mix_buf_r[i] = fclamp(mix_buf_r[i] , -1.0f, 1.0f);
       mix_buf_l[i] = fast_tanh( mix_buf_l[i]); // saturator
       mix_buf_r[i] = fast_tanh( mix_buf_r[i]);
   }
#ifdef DEBUG_MASTER_OUT
  meter *= 0.95f;
  meter += abs(mono_mix); 
  DEBF("out= %0.5f\r\n", meter);
#endif
}


inline void i2s_output () {
  // now out_buf is ready, output
  if (processing) {
  #ifdef USE_INTERNAL_DAC
    for (int i=0; i < DMA_BUF_LEN; i++) {      
      out_buf._unsigned[i*2] = (uint16_t)(127.0f * ( fast_tanh( mix_buf_l[i]) + 1.0f)) << 8U; // 256 output levels is way to little
      out_buf._unsigned[i*2+1] = (uint16_t)(127.0f * ( fast_tanh( mix_buf_r[i]) + 1.0f)) << 8U ; // maybe you'll be lucky to fully use this range
    }
    i2s_write(i2s_num, out_buf._unsigned, sizeof(out_buf._unsigned), &bytes_written, portMAX_DELAY);
  #else
    for (int i=0; i < DMA_BUF_LEN; i++) {      
      out_buf._signed[i*2] = 0x7fff * (float)(fast_tanh( mix_buf_l[i])) ; 
      out_buf._signed[i*2+1] = 0x7fff * (float)(fast_tanh( mix_buf_r[i])) ;
    }
    i2s_write(i2s_num, out_buf._signed, sizeof(out_buf._signed), &bytes_written, portMAX_DELAY);
  #endif
  }
}


inline float lookupTable(float (&table)[TABLE_SIZE], float index ) { // lookup value in a table by float index, using linear interpolation
  static float v1, v2, res;
  static int32_t i;
  static float f;
  i = (int32_t)index;
  f = index - i;
  v1 = (table)[i];
  v2 = (table)[i+1];
  res = (float)f * (float)(v2-v1) + v1;
  return res;
}

inline float fclamp(float in, float min, float max){
    return fmin(fmax(in, min), max);
}

inline float fast_tanh(float x){
  //return tanh(x);
    float sign = 1.0f;
    if (x<0.0f) {
        sign=-1.0f;
        x= -x;
    }
    if (x>=4.95f) {
      return sign; // tanh(x) ~= 1, when |x| > 4
    }
  //  if (x<=0.4f) return float(x*sign) * 0.9498724f; // smooth region borders; tanh(x) ~= x, when |x| < 0.4 
    return  sign * lookupTable(tanh_tbl, (x*TANH_LOOKUP_COEF)); // lookup table contains tanh(x), 0 <= x <= 5
 //  float poly = (2.12-2.88*x+4.0*x*x);
 //  return sign * x * (poly * one_div(poly * x + 1.0f)); // very good approximation found here https://www.musicdsp.org/en/latest/Other/178-reasonably-accurate-fastish-tanh-approximation.html
                                                    // but it uses float division which is not that fast on esp32
}

static __attribute__((always_inline)) inline float one_div(float a) {
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


inline float linToLin(float in, float inMin, float inMax, float outMin, float outMax)
{
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) * one_div(inMax-inMin);

  // map the tmp-value to the range outMin...outMax:
  tmp *= (outMax-outMin);
  tmp += outMin;

  return tmp;
}

inline float linToExp(float in, float inMin, float inMax, float outMin, float outMax)
{
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) * one_div(inMax-inMin);

  // map the tmp-value exponentially to the range outMin...outMax:
  //tmp = outMin * exp( tmp*(log(outMax)-log(outMin)) );
  return outMin * expf( tmp*(logf(outMax * one_div(outMin))) );
}


inline float expToLin(float in, float inMin, float inMax, float outMin, float outMax)
{
  float tmp = logf(in * one_div(inMin)) * one_div( logf(inMax * one_div(inMin)));
  return outMin + tmp * (outMax-outMin);
}

inline float knobMap(float in, float outMin, float outMax) {
  return outMin + lookupTable(knob_tbl, (int)(in * TABLE_SIZE)) * (outMax - outMin);
}
