#include "moogladder.h"

inline float MoogLadder::my_tanh(float x)
{
  //return tanh(x);
    float sign = 1.0f;
    float poly;
    if (x<0.0f) {
        sign=-1.0f;
        x= -x;
    }
    if (x>=4.95f) {
      return sign;
    }
    if (x<=0.4f) return float(x*sign) * 0.9498724f; // smooth region borders    
    return  sign * lookupTable(tanh_tbl,(x*TANH_LOOKUP_COEF)); // lookup table, 5 is max argument value 
 //  poly = (2.12-2.88*x+4.0*x*x);
 //  return sign * x * (poly / (poly * x + 1.0f)); // very good approximation found here https://www.musicdsp.org/en/latest/Other/178-reasonably-accurate-fastish-tanh-approximation.html
                                                    // but it uses float division which is not that fast on esp32
  //  return sign * tanh(x);  // this version uses native tanh() which makes it slow in some cases
}

/*
inline float MoogLadder::my_tanh(float x)
  {
    float a = fabs(2*x);
    float b = 24+a*(12+a*(6+a));
    return 2*(x*b)/(a*b+48);
  }
*/

void MoogLadder::Init(float sample_rate) {
    sample_rate_  = sample_rate;
    one_sr_       = 1.0 / sample_rate;
    istor_        = 0.0f;
    res_          = 0.4f;
    freq_         = 1000.0f;
    drive_        = 0.001f;
    compens_      = (drive_*0.85f+3.2f)/drive_;
    for(int i = 0; i < 6; i++)
    {
        delay_[i]       = 0.0;
        tanhstg_[i % 3] = 0.0;
    }

    old_freq_ = 0.0f;
    old_res_  = -1.0f;
}

float MoogLadder::Process(float in) {
    float  freq = freq_;
    float  res  = res_;
    float  res4;
    float* delay   = delay_;
    float* tanhstg = tanhstg_;
    float  stg[4];
    float  tune;

    static float THERMAL = 0.000025;
    static float ONE_THERMAL = 40000.0f;
    in *= drive_ ; // saturator ?
    if(res < 0)
    {
        res = 0;
    }

    if (old_freq_ != freq || old_res_ != res) {
        float f, fc, fc2, fc3, fcr;
        old_freq_ = freq;
        fc        = (freq * one_sr_);
        f         = 0.5f * fc;
        fc2       = fc * fc;
        fc3       = fc2 * fc;

        fcr  = 1.8730f * fc3 + 0.4955f * fc2 - 0.6490f * fc + 0.9988f;
        acr  = -3.9364f * fc2 + 1.8409f * fc + 0.9968f;
        tune = (1.0f - expf(-(TWOPI * f * fcr))) * ONE_THERMAL;

        old_res_  = res;
        old_acr_  = acr;
        old_tune_ = tune;
    } else {
        res  = old_res_;
        acr  = old_acr_;
        tune = old_tune_;
    }

    res4 = 4.0f * res * acr;

    for(int j = 0; j < 2; j++) {
        in -= res4 * delay[5];
        delay[0] = stg[0]
            = delay[0] + tune * (my_tanh(in * THERMAL) - tanhstg[0]);
        for(int k = 1; k < 4; k++)
        {
            in     = stg[k - 1];
            stg[k] = delay[k]
                     + tune
                           * ((tanhstg[k - 1] = my_tanh(in * THERMAL))
                              - (k != 3 ? tanhstg[k]
                                        : my_tanh(delay[k] * THERMAL)));
            delay[k] = stg[k];
        }
        delay[5] = (stg[3] + delay[4]) * 0.5f;
        delay[4] = stg[3];
    }
    return (float)(my_tanh(delay[5]*2.0f) * (float)compens_);
}
