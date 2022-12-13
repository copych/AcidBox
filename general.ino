static void drums() {
  // float fl_sample = 0.0f;
  // float fr_sample = 0.0f;
    for (int i=0; i < DMA_BUF_LEN; i++){
      Drums.Process( &drums_buf_l[i], &drums_buf_r[i] );
    } 
}

static void mixer() { // sum buffers 
  float synth1_out_l, synth1_out_r, synth2_out_l, synth2_out_r, drums_out_l, drums_out_r;
  float dly_l, dly_r, rvb_l, rvb_r;
    dly_k1 = Synth1._sendDelay;
    dly_k2 = Synth2._sendDelay;
    dly_k3 = Drums._sendDelay;
    rvb_k1 = Synth1._sendReverb;
    rvb_k2 = Synth2._sendReverb;
    rvb_k3 = Drums._sendReverb;
    for (int i=0; i < DMA_BUF_LEN; i++) { 
      synth1_out_l = Synth1.pan*synth_buf[0][i];
      synth1_out_r = (1.0f-Synth1.pan)*synth_buf[0][i];
      synth2_out_l = Synth2.pan*synth_buf[1][i];
      synth2_out_r = (1.0f-Synth2.pan)*synth_buf[1][i];
      drums_out_l = drums_buf_l[i];
      drums_out_r = drums_buf_r[i];
      
      dly_l = dly_k1 * synth1_out_l + dly_k2 * synth2_out_l + dly_k3 * drums_out_l; // delay bus
      dly_r = dly_k1 * synth1_out_r + dly_k2 * synth2_out_r + dly_k3 * drums_out_r;
      Delay.Process( &dly_l, &dly_r );
      
      rvb_l = rvb_k1 * synth1_out_l + rvb_k2 * synth2_out_l + rvb_k3 * drums_out_l; // reverb bus
      rvb_r = rvb_k1 * synth1_out_r + rvb_k2 * synth2_out_r + rvb_k3 * drums_out_r;
      Reverb.Process( &rvb_l, &rvb_r );
      
      mix_buf_l[i] = 0.2f * (synth1_out_l + synth2_out_l + drums_out_l + dly_l + rvb_l);
      mix_buf_r[i] = 0.2f * (synth1_out_r + synth2_out_r + drums_out_r + dly_r + rvb_r);

      Comp.Process(0.5*(mix_buf_l[i] + mix_buf_r[i])); // calculate gain based on a mono mix, can be a side chain
      mix_buf_l[i] = Comp.Apply(mix_buf_l[i]);
      mix_buf_r[i] = Comp.Apply(mix_buf_r[i]);
      
   //   mix_buf_l[i] = fclamp(mix_buf_l[i] , -1.0f, 1.0f); // clipper
    //  mix_buf_r[i] = fclamp(mix_buf_r[i] , -1.0f, 1.0f);
       mix_buf_l[i] = fast_tanh( mix_buf_l[i]);
       mix_buf_r[i] = fast_tanh( mix_buf_r[i]);
   }
}



inline void i2s_output () {  
  for (int i=0; i < DMA_BUF_LEN; i++) {      
      out_buf._signed[i*2] = 0x8000 * ( mix_buf_l[i]);
      out_buf._signed[i*2+1] = 0x8000 * ( mix_buf_r[i]);
  }
  // now out_buf is ready, output

  //i2s_write(i2s_num, out_buf._unsigned, sizeof(out_buf._unsigned), &bytes_written, portMAX_DELAY); // NO-DAC case
  i2s_write(i2s_num, out_buf._signed, sizeof(out_buf._signed), &bytes_written, portMAX_DELAY);
}


/** quick fp clamp
*/
inline float fclamp(float in, float min, float max)
{
    return fmin(fmax(in, min), max);
}

inline float fast_tanh(float x)
{
  //return tanh(x);
    float sign = 1.0f;
    if (x<0.0f) {
        sign=-1.0f;
        x= -x;
    }
    if (x>=4.95f) {
      return sign;
    }
    if (x<=0.4f) return float(x*sign) * 0.9498724f; // smooth region borders    
    return  sign * tanh_2048[(uint16_t)(x*409.6f)]; // lookup table 2048 / 5 = 409.6
 //  return sign * x/(x+1.0/(2.12-2.88*x+4.0*x*x)); // very good approximation for tanh() found here https://www.musicdsp.org/en/latest/Other/178-reasonably-accurate-fastish-tanh-approximation.html
  //  return sign * tanh(x);
}
