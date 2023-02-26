
float noteToFreq(int note) {
    return (440.0f / 32.0f) * pow(2, ((float)(note - 9) / 12.0));
}

float expSaw_fill(uint16_t i) { // this one contains a piece of exp(-x) normalized to fit into [-1.0 .. 1.0] , "saw", "square" are generated basing on this table
  float x = (float)i * 2048.0f / (float)TABLE_SIZE;
  float res = exp((float)(-x)/486.0f) * 2.03f - 1.03f;
  return res;
}

float knob_fill(uint16_t i) { // f(x) = (exp(k*x)-1)/b, 0 <= x <= 1, 0 <= f(x) <= 1, x mapped to [0 .. TABLE_SIZE]
  float x = (float)i / (float)TABLE_SIZE;
  float res = ( expf((float)(x * 2.71f))-1.0f) / 14.02928f;
  DEBUG(x);
  return res;
}

float tanh_fill(uint16_t i) {
  float res = tanh( (float)i * TANH_LOOKUP_MAX / (float)TABLE_SIZE); // 0.0 -- 5.0 argument
  return res;
}

float expSquare_fill(uint16_t i) { // requires exp() table
  uint16_t j = i + TABLE_SIZE/2;
  if (j>=TABLE_SIZE) j = j - TABLE_SIZE;
  float res = 0.685f * (exp_tbl[i]-exp_tbl[j]);
  return res;
}

float freqToPhaseInc(float freq, uint16_t sampleSize, uint16_t sampleRate) {
  return freq * (float)sampleSize / (float)sampleRate;
}

void buildTables() {
  for (uint8_t i = 0 ; i<128; ++i) {
    midi_pitches[i] = noteToFreq(i);
    midi_phase_steps[i] = noteToFreq(i) * PI * 2.0f / (float)SAMPLE_RATE;
    midi_tbl_steps[i] = noteToFreq(i) * (float)TABLE_SIZE / (float)SAMPLE_RATE;
  }

  // one extra value is tor linear 'extrapolation'
  for (uint16_t i = 0; i < TABLE_SIZE; i++) {
    exp_tbl[i] = expSaw_fill(i);
  }

  for (uint16_t i = 0; i < TABLE_SIZE; i++) {
    exp_square_tbl[i] = expSquare_fill(i);
    tanh_tbl[i] = tanh_fill(i); 
    knob_tbl[i] = knob_fill(i); 
  //  saw_tbl[i] = 1.0f - 2.0f * (float)i / (float)TABLE_SIZE;
  //  square_tbl[i] = (i>TABLE_SIZE/2) ? 1.0f : -1.0f; 
  }
}
