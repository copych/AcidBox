#pragma once

#ifndef FXFCR_H
#define FXFCR_H
/*
   this file includes some simple effects
   - dual filter
   - bit crusher

   Author: Marcel Licence
*/

#define WAVEFORM_BIT  10UL
#define WAVEFORM_CNT  (1<<WAVEFORM_BIT)
#define WAVEFORM_Q4   (1<<(WAVEFORM_BIT-2))
#define WAVEFORM_MSK  ((1<<WAVEFORM_BIT)-1)
#define WAVEFORM_I(i) ((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK

class FxFilterCrusher {
  public:
    FxFilterCrusher() {}

    void Init (float samplerate) {
      Init();
    }
    void Init( void ) {
      for ( int i = 0; i < WAVEFORM_CNT; i++ ) {
        float val = (float)sin(i * 2.0 * PI / WAVEFORM_CNT);
        sine[i] = val;
      }

      mainFilterL_LP.filterCoeff = &filterGlobalC_LP;
      mainFilterR_LP.filterCoeff = &filterGlobalC_LP;
      mainFilterL_HP.filterCoeff = &filterGlobalC_HP;
      mainFilterR_HP.filterCoeff = &filterGlobalC_HP;
    };

    inline float Process (float sample) {
      Process(&sample, &sample);
      return sample;
    }

    void Process( float* left, float* right );

    void SetCutoff( float value ) {
      highpassC = value >= 0.5 ? (value - 0.5f) * 2.0f : 0.0f;
      lowpassC = value <= 0.5 ? (value) * 2.0f : 1.0f;
#ifdef DEBUG_FX
      DEBF("Filter TP: %0.2f, HP: %02f\n", lowpassC, highpassC);
#endif
    };

    void SetResonance( float value ) {
      filtReso =  0.5f + 10 * value * value * value; /* min q is 0.5 here */
#ifdef DEBUG_FX
      DEBF("main filter reso: %0.3f\n", filtReso);
#endif
    };

    void SetBitCrusher( float value ) {
      bitCrusher = pow(2, -32.0f * value);
#ifdef DEBUG_FX
      DEBF("main filter bitCrusher: %0.3f\n", bitCrusher);
#endif
    };

  private:

    struct filterCoeffT {
      float aNorm[2] = {0.0f, 0.0f};
      float bNorm[3] = {1.0f, 0.0f, 0.0f};
    };

    struct filterProcT {
      struct filterCoeffT *filterCoeff;
      float w[3];
    };

    struct filterCoeffT filterGlobalC_LP, filterGlobalC_HP;
    struct filterProcT mainFilterL_LP, mainFilterR_LP, mainFilterL_HP, mainFilterR_HP;

    float sine[WAVEFORM_CNT];

    float highpassC = 0.0f;
    float lowpassC = 1.0f;
    float filtReso = 1.0f;

    float cutoff_hp_slow = 0.0f;
    float cutoff_lp_slow = 1.0f;

    uint8_t effect_prescaler = 0;

    float bitCrusher = 1.0f;

    // calculate coefficients of the 2nd order IIR filter

    inline void Filter_CalculateTP(float c, float reso, struct filterCoeffT *const  filterC );

    inline void Filter_CalculateHP(float c, float reso, struct filterCoeffT *const  filterC );

    inline void Filter_Process( float *const signal, struct filterProcT *const filterP ) {
      const float out = filterP->filterCoeff->bNorm[0] * (*signal) + filterP->w[0];
      filterP->w[0] = filterP->filterCoeff->bNorm[1] * (*signal) - filterP->filterCoeff->aNorm[0] * out + filterP->w[1];
      filterP->w[1] = filterP->filterCoeff->bNorm[2] * (*signal) - filterP->filterCoeff->aNorm[1] * out;
      *signal = out;
    };



};
#endif
