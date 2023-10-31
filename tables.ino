
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

float tanh_fill(uint16_t i) {
  float res = tanh( (float)i * TANH_LOOKUP_MAX * (float)DIV_TABLE_SIZE); // 0.0 -- 5.0 argument
  return res;
}

float sin_fill(uint16_t i) {
  float res = sinf( (float)i * TWOPI * (float)DIV_TABLE_SIZE ); // 0.0 -- 2*pi argument
  return res;
}

float expSquare_fill(uint16_t i) { // requires exp() table
  uint16_t j = i + 0.5f*TABLE_SIZE;
  if (j>=TABLE_SIZE) j = j - TABLE_SIZE;
  float res = 0.66f * (saw_tbl[i]-saw_tbl[j]);
  return res;
}

float freqToPhaseInc(float freq, uint16_t sampleSize, uint16_t sampleRate) {
  return freq * (float)sampleSize / (float)sampleRate;
}

void buildTables() {
  for (int i = 0 ; i<128; ++i) {
    midi_pitches[i] = noteToFreq(i);
    midi_phase_steps[i] = noteToFreq(i) * PI * 2.0f * (float)DIV_SAMPLE_RATE;
    midi_tbl_steps[i] = noteToFreq(i) * (float)TABLE_SIZE * (float)DIV_SAMPLE_RATE;
  }

  for (int i = 0; i <= TABLE_SIZE; i++) {
    exp_tbl[i] = exp_fill(i);
    saw_tbl[i] = expSaw_fill(i);
  }

  for (int i = 0; i <= TABLE_SIZE; i++) {
    exp_square_tbl[i] = expSquare_fill(i);
    tanh_tbl[i] = tanh_fill(i); 
    knob_tbl[i] = knob_fill(i);
	  sin_tbl[i]  = sin_fill(i);
  //  saw_tbl[i] = 1.0f - 2.0f * (float)i * (float)DIV_TABLE_SIZE;
  //  square_tbl[i] = (i>TABLE_SIZE/2) ? 1.0f : -1.0f; 
  }
  for (int i = 0; i < 16; i++) {
    for (int j = 0; j < 16; j++){
      norm1_tbl[i][j] = cutoff_reso_avg + (cutoff_reso[i][j] - cutoff_reso_avg) * NORM1_DEPTH;
      norm2_tbl[i][j] = wfolder_overdrive_avg + (wfolder_overdrive[i][j] - wfolder_overdrive_avg) * NORM2_DEPTH;
    }
  }
}
