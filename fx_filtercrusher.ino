#include "fx_filtercrusher.h"


void FxFilterCrusher::SetCutoff( float value ) {
  highpassC = value >= 0.5 ? (value - 0.5f) * 2.0f : 0.0f;
  lowpassC = value <= 0.5 ? (value) * 2.0f : 1.0f;
#ifdef DEBUG_FX
  DEBF("Filter TP: %0.6f, HP: %06f\n", lowpassC, highpassC);
#endif
};

void FxFilterCrusher::SetResonance( float value ) {
  filtReso =  0.5f + 10 * value * value * value; /* min q is 0.5 here */
  div_2_reso = one_div(2.0f * filtReso);
#ifdef DEBUG_FX
  DEBF("main filter reso: %0.3f\n", filtReso);
#endif
};


void FxFilterCrusher::SetBitCrusher( float value ) {
  bitCrusher = pow(2, -32.0f * value);
  div_bitCrusher = 1.0f * one_div(bitCrusher);
#ifdef DEBUG_FX
  DEBF("main filter bitCrusher: %0.3f\n", bitCrusher);
#endif
};

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
};


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
  fast_sincos(omega , &sinOmega, &cosOmega);
  alpha = sinOmega * one_div_2_reso;
  b[0] = (1 - cosOmega) * 0.5f;
  b[1] = 1 - cosOmega;
  b[2] = b[0];
  a[0] = 1 + alpha;
  a[1] = -2 * cosOmega;
  a[2] = 1 - alpha;

  // Normalize filter coefficients
  float factor = one_div(a[0]);

  aNorm[0] = a[1] * factor;
  aNorm[1] = a[2] * factor;

  bNorm[0] = b[0] * factor;
  bNorm[1] = b[1] * factor;
  bNorm[2] = b[2] * factor;
};

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

  fast_sincos(omega , &sinOmega, &cosOmega);
  alpha = sinOmega * one_div_2_reso;
  b[0] = (1 + cosOmega) * 0.5f;
  b[1] = -(1 + cosOmega);
  b[2] = b[0];
  a[0] = 1 + alpha;
  a[1] = -2 * cosOmega;
  a[2] = 1 - alpha;

  // Normalize filter coefficients
  float factor = one_div(a[0]) ;

  aNorm[0] = a[1] * factor;
  aNorm[1] = a[2] * factor;

  bNorm[0] = b[0] * factor;
  bNorm[1] = b[1] * factor;
  bNorm[2] = b[2] * factor;
};
