
float noteToFreq(int note) {
    return (440.0f / 32.0f) * pow(2, ((float)(note - 9) / 12.0));
}

float expSaw2048(uint16_t i) {
  float res = exp((float)(-i)/1024.0f) * 2.3f - 1.31f;
  return res;
}

float expSquare2048(uint16_t i) { // requires saw table
  uint16_t j = i + WAVE_SIZE/2;
  if (j>=WAVE_SIZE) j = j - WAVE_SIZE;
  float res = 0.685f * (saw_2048[i]-saw_2048[j]);
  return res;
}

float freqToPhaseInc(float freq, uint16_t sampleSize, uint16_t sampleRate) {
  return freq * (float)sampleSize / (float)sampleRate;
}

void buildTables() {
  for (uint8_t i = 0 ; i<128; ++i) {
    midi_pitches[i] = noteToFreq(i);
    midi_phase_steps[i] = noteToFreq(i) * PI * 2.0f / (float)SAMPLE_RATE;
    midi_2048_steps[i] = noteToFreq(i) * (float)WAVE_SIZE / (float)SAMPLE_RATE;
  }
  for (uint16_t i = 0; i < WAVE_SIZE; i++) {
    saw_2048[i] = expSaw2048(i);
  }
  
  for (uint16_t i = 0 ; i < WAVE_SIZE; i++) {
    square_2048[i] = expSquare2048(i);
  }
}

/** quick fp clamp
*/
inline float fclamp(float in, float min, float max)
{
    return fmin(fmax(in, min), max);
}
