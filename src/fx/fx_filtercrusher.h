#pragma once
#ifndef FXFCR_H
#define FXFCR_H

#include "Arduino.h"
#include "../../general.h"

/*
   this file includes some simple effects
   - dual filter
   - bit crusher

   Author: Marcel Licence
*/
/*
#define WAVEFORM_BIT  10UL
#define WAVEFORM_CNT  (1<<WAVEFORM_BIT)
#define WAVEFORM_MSK  ((1<<WAVEFORM_BIT)-1)
#define WAVEFORM_I(i) ((i) >> (32 - WAVEFORM_BIT)) & WAVEFORM_MSK
*/

class FxFilterCrusher {
  public:
    FxFilterCrusher() {}

    void Init (float samplerate) {
      Init();
    }
    void Init( void ) {
  /*    for ( int i = 0; i < WAVEFORM_CNT; i++ ) {
        float val = (float)sin(i * 2.0f * PI / WAVEFORM_CNT);
        sine[i] = val;
      }
*/
      mainFilterL_LP.filterCoeff = &filterGlobalC_LP;
      mainFilterR_LP.filterCoeff = &filterGlobalC_LP;
      mainFilterL_HP.filterCoeff = &filterGlobalC_HP;
      mainFilterR_HP.filterCoeff = &filterGlobalC_HP;
    };

    inline float Process (float sample);

    inline void Process( float* left, float* right );

    void SetCutoff( float value ) ;

    void SetResonance( float value ) ;

    void SetBitCrusher( float value ) ;

  private:

    struct filterCoeffT {
      float aNorm[2] = {0.0f, 0.0f};
      float bNorm[3] = {1.0f, 0.0f, 0.0f};
    };

    struct filterProcT {
      struct filterCoeffT *filterCoeff;
      float w[2];
    };

    struct filterCoeffT filterGlobalC_LP, filterGlobalC_HP;
    struct filterProcT mainFilterL_LP, mainFilterR_LP, mainFilterL_HP, mainFilterR_HP;

 //   float sine[WAVEFORM_CNT];

    float highpassC = 0.0f;
    float lowpassC = 1.0f;
    float filtReso = 1.0f;

    float div_2_reso = 0.5f;

    float cutoff_hp_slow = 0.0f;
    float cutoff_lp_slow = 1.0f;

    uint8_t effect_prescaler = 0;

    float bitCrusher = 1.0f;
    float div_bitCrusher = 1.0f;

    // calculate coefficients of the 2nd order IIR filter

    inline void Filter_CalculateTP(float c, float one_div_2_reso, struct filterCoeffT *const  filterC );

    inline void Filter_CalculateHP(float c, float one_div_2_reso, struct filterCoeffT *const  filterC );

    inline void Filter_Process( float *const signal, struct filterProcT *const filterP );
};

inline float FxFilterCrusher::Process (float sample) {
  Process(&sample, &sample);
  return sample;
}

void FxFilterCrusher::Process( float* left, float* right ) {
  float oldLP, oldHP, oldReso;
  effect_prescaler++;

  Filter_Process(left, &mainFilterL_LP);
  Filter_Process(right, &mainFilterR_LP);
  Filter_Process(left, &mainFilterL_HP);
  Filter_Process(right, &mainFilterR_HP);
/*
  cutoff_lp_slow = (float)cutoff_lp_slow * 0.99f + 0.01f * ((float)lowpassC - (float)cutoff_lp_slow);
  cutoff_hp_slow = (float)cutoff_hp_slow * 0.99f + 0.01f * ((float)highpassC - (float)cutoff_hp_slow);
*/

  cutoff_lp_slow =  lowpassC ;
  cutoff_hp_slow =  highpassC  ;
  /* we can not calculate in each cycle */
  if ( effect_prescaler % 16 == 0 ) {
    if ( filtReso != oldReso || oldHP != highpassC || oldLP != lowpassC ) {
      Filter_CalculateTP(cutoff_lp_slow, div_2_reso, &filterGlobalC_LP);
      Filter_CalculateHP(cutoff_hp_slow, div_2_reso, &filterGlobalC_HP);
      oldLP = lowpassC;
      oldHP = highpassC;
      oldReso = filtReso;    
    }
  }


  if ( bitCrusher < 1.0f ) {
    int32_t ul = *left * (float)bitCrusher * (float)(1 << 29);
    *left = ((float)ul * div_bitCrusher * (float)(1 << 29));

    int32_t ur = *right * (float)bitCrusher * (float)(1 << 29);
    *right = ((float)ur * div_bitCrusher * (float)(1 << 29));
  }
}

inline void FxFilterCrusher::Filter_CalculateTP(float c, float one_div_2_reso, struct filterCoeffT *const  filterC ) {
  float *aNorm = filterC->aNorm;
  float *bNorm = filterC->bNorm;

  float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

  // change curve of cutoff a bit
  // maybe also log or exp function could be used

  c = (float)(c * c * c);

  if (c > 0.9975f ) {
    omega = 0.9975f;
  } else if ( c < 0.0025f ) {
    omega = 0.0025f;
  } else {
    omega = c;
  }

  // omega = fast_shape(4.0f * c - 2.0f) * 0.5f + 0.5f; // it's smooth and sounds badly
  
  // use lookup here to get quicker results
  /*
  cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
  sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];
*/
  General::fast_sincos(omega , &sinOmega, &cosOmega);
  alpha = sinOmega * one_div_2_reso;
  b[0] = (1 - cosOmega) * 0.5f;
  b[1] = 1 - cosOmega;
  b[2] = b[0];
  a[0] = 1 + alpha;
  a[1] = -2 * cosOmega;
  a[2] = 1 - alpha;

  // Normalize filter coefficients
  float factor = General::one_div(a[0]);

  aNorm[0] = a[1] * factor;
  aNorm[1] = a[2] * factor;

  bNorm[0] = b[0] * factor;
  bNorm[1] = b[1] * factor;
  bNorm[2] = b[2] * factor;
}

inline void FxFilterCrusher::Filter_CalculateHP(float c, float one_div_2_reso, struct filterCoeffT *const  filterC ) {
  float *aNorm = filterC->aNorm;
  float *bNorm = filterC->bNorm;
  float  cosOmega, omega, sinOmega, alpha, a[3], b[3];

  // change curve of cutoff a bit
  // maybe also log or exp function could be used

  c = (float)(c * c * c);

  if (c > 0.9975f ) {
    omega = 0.9975f;
  } else if ( c < 0.0025f ) {
    omega = 0.0025f;
  } else {
    omega = c;
  }

  //omega = fast_shape(4.0f * c - 2.0f) * 0.5f + 0.5f;
  // use lookup here to get quicker results
/*
  cosOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega + (float)((1ULL << 30) - 1)))];
  sinOmega = sine[WAVEFORM_I((uint32_t)((float)((1ULL << 31) - 1) * omega))];
*/

  General::fast_sincos(omega , &sinOmega, &cosOmega);
  alpha = sinOmega * one_div_2_reso;
  b[0] = (1 + cosOmega) * 0.5f;
  b[1] = -(1 + cosOmega);
  b[2] = b[0];
  a[0] = 1 + alpha;
  a[1] = -2 * cosOmega;
  a[2] = 1 - alpha;

  // Normalize filter coefficients
  float factor = General::one_div(a[0]) ;

  aNorm[0] = a[1] * factor;
  aNorm[1] = a[2] * factor;

  bNorm[0] = b[0] * factor;
  bNorm[1] = b[1] * factor;
  bNorm[2] = b[2] * factor;
}

inline void FxFilterCrusher::Filter_Process(float *const signal, struct filterProcT *const filterP) {
  const float out = filterP->filterCoeff->bNorm[0] * (*signal) + filterP->w[0];
  filterP->w[0] = filterP->filterCoeff->bNorm[1] * (*signal) - filterP->filterCoeff->aNorm[0] * out + filterP->w[1];
  filterP->w[1] = filterP->filterCoeff->bNorm[2] * (*signal) - filterP->filterCoeff->aNorm[1] * out;
  *signal = out;
}

#endif
