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
  div_2_reso = General::one_div(2.0f * filtReso);
#ifdef DEBUG_FX
  DEBF("main filter reso: %0.3f\n", filtReso);
#endif
};

void FxFilterCrusher::SetBitCrusher( float value ) {
  bitCrusher = pow(2, -32.0f * value);
  div_bitCrusher = 1.0f * General::one_div(bitCrusher);
#ifdef DEBUG_FX
  DEBF("main filter bitCrusher: %0.3f\n", bitCrusher);
#endif
};