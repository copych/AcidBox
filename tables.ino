#include "tables.h"

float noteToFreq(int note) {
    return (440.0f / 32.0f) * pow(2, ((float)(note - 9) / 12.0));
}

float expSaw_fill(uint16_t i) { // this one contains a piece of exp(-x) normalized to fit into [-1.0 .. 1.0] , "saw", "square" are generated basing on this table
  float x = (float)i * 2048.0f * (float)DIV_TABLE_SIZE;
  float res = exp((float)(-x)*0.0011111111f) * 2.229f - 1.229f;
  return res;
}

float exp_fill(uint16_t i) { // this waveform is for envelopes shaping, it has more exp() effect
  float x = (float)i * 2048.0f * (float)DIV_TABLE_SIZE;
  float res = exp((float)(-x)*0.002057613168f) * 2.03f - 1.03f;
  return res;
}

float knob_fill(uint16_t i) { // f(x) = (exp(k*x)-1)/b, 0 <= x <= 1, 0 <= f(x) <= 1, x mapped to [0 .. TABLE_SIZE]
  float x = (float)i * (float)DIV_TABLE_SIZE;
  float res = ( expf((float)(x * 2.71f))-1.0f) * 0.071279495455219f;
  return res;
}

float shaper_fill(uint16_t i) {
  float x = (float)i * SHAPER_LOOKUP_MAX * (float)DIV_TABLE_SIZE; // argument belongs [ 0 .. 5 ]
#ifdef SHAPER_USE_TANH
  float res = tanh( x ); 
#endif
#ifdef SHAPER_USE_CUBIC
  x = fclamp(x, -1.4142, 1.4142);
  float res = x - (x * x * x / 6.8283);
#endif
  return res;
}

float sin_fill(uint16_t i) {
  float res = sinf( (float)i * TWOPI * (float)DIV_TABLE_SIZE ); // 0.0 -- 2*pi argument
  return res;
}

float expSquare_fill(uint16_t i) { // requires exp() table
  uint16_t j = i + 0.5f*TABLE_SIZE;
  if (j>=TABLE_SIZE) j = j - TABLE_SIZE;
  float res = 0.66f * (Tables::saw_tbl[i]-Tables::saw_tbl[j]);
  return res;
}

float freqToPhaseInc(float freq, uint16_t sampleSize, uint16_t sampleRate) {
  return freq * (float)sampleSize / (float)sampleRate;
}

void buildTables() {
  for (int i = 0 ; i<128; ++i) {
    Tables::midi_tbl_steps[i] = noteToFreq(i) * (float)TABLE_SIZE * (float)DIV_SAMPLE_RATE;
  }

  for (int i = 0; i <= TABLE_SIZE; i++) {
    Tables::exp_tbl[i] = exp_fill(i);
    Tables::saw_tbl[i] = expSaw_fill(i);
  }

  for (int i = 0; i <= TABLE_SIZE; i++) {
    Tables::exp_square_tbl[i] = expSquare_fill(i);
    Tables::shaper_tbl[i] = shaper_fill(i); 
    Tables::knob_tbl[i] = knob_fill(i);
	  Tables::sin_tbl[i]  = sin_fill(i);
  }
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++){
      Tables::norm1_tbl[i][j] = cutoff_reso_avg + (cutoff_reso[i][j] - cutoff_reso_avg) * NORM1_DEPTH;
      Tables::norm2_tbl[i][j] = wfolder_overdrive_avg + (wfolder_overdrive[i][j] - wfolder_overdrive_avg) * NORM2_DEPTH;
    }
  }
}