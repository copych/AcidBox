#include "synthvoice.h"

SynthVoice::SynthVoice(uint8_t index, volatile int *current_gen_buf) {
  _index = index;
  _current_gen_buf = current_gen_buf;
}

void SynthVoice::Init() {
  _envMod = 0.5f;
  _accentLevel = 0.5f;
  _cutoff = 0.2f; // 0..1 normalized freq range. Keep in mind that EnvMod set to max practically doubles this range
  _filter_freq = General::linToExp(_cutoff, 0.0f, 1.0f, MIN_CUTOFF_FREQ, MAX_CUTOFF_FREQ);
  _reso = 0.4f;
  _gain = 0.0f; // values >1 will distort sound
  _drive = 0.0f;
  //  midiNotes[0] = -1;
  //  midiNotes[1] = -1;
  _midiNote = 69;
  _currentStep = 1.0f;
  _targetStep = 1.0f;
  _tuning = 1.0f;
  _pitchbend = 1.0f;
  _deltaStep = 0.0f;
  _slideMs = 60.0f;
  _phaze = 0.0f;
  mva1.n = 0 ;
  _pan = 0.5;

  AmpEnv.init(SAMPLE_RATE);
  FltEnv.init(SAMPLE_RATE);
  // parameters of envelopes
  _ampAttackMs = 0.5;
  _ampDecayMs = 1230.0;
  _ampReleaseMs = 1.0;
  _filterAttackMs = 3.0;
  _filterDecayMs = 1000.0;
  _filterAccentAttackMs = 30.0;
  _filterAccentDecayMs = 200.0;
  
  Distortion.Init();
  Drive.Init();

  Filter.Init((float)SAMPLE_RATE);
  
#if FILTER_TYPE == 2
//  Filter.SetMode(TeeBeeFilter::LP_18);
  Filter.SetMode(TeeBeeFilter::TB_303);
#endif
  highpass1.setMode(OnePoleFilter::HIGHPASS);
  highpass1.setCutoff(44.486f);
  highpass2.setMode(OnePoleFilter::HIGHPASS);
  highpass2.setCutoff(24.167f);
  allpass.setMode(OnePoleFilter::ALLPASS);
  allpass.setCutoff(14.008f);
  ampDeclicker.setMode(BiquadFilter::LOWPASS12);
  ampDeclicker.setGain(General::amp2dB(sqrt(0.5f)) );
  ampDeclicker.setFrequency(200.0f);
  filtDeclicker.setMode(BiquadFilter::LOWPASS12);
  filtDeclicker.setGain(General::amp2dB(sqrt(0.5f)) );
  filtDeclicker.setFrequency(200.0f);
  notch.setMode(BiquadFilter::BANDREJECT);
  notch.setFrequency(7.5164f);
  notch.setBandwidth(4.7f);
}

void IRAM_ATTR SynthVoice::generate() {
    for (int i=0; i < DMA_BUF_LEN; i++){
      synth_buf[*_current_gen_buf][i] = getSample();      
    } 
}

// The following code initially written by Anton Savov,
// is taken from http://antonsavov.net/cms/projects/303andmidi.html
// Monophonic Voice Allocator (with Accent, suitable for the 303)
// "Newest" note-priority rule
// Modified version, allows multiple Notes with the same pitch

void SynthVoice::mva_note_on(mva_data *p, uint8_t note, uint8_t accent)
{
  uint8_t s = 0;
  uint8_t i = 0;

  // shift all notes back
  uint8_t m = p->n + 1;
  m = (m > MIDI_MVA_SZ ? MIDI_MVA_SZ : m);
  s = m;
  i = m;
  while (i > 0)
  {
    --s;
    p->notes[i] = p->notes[s];
    p->accents[i] = p->accents[s];
    i = s;
  }
  // put the new note first
  p->notes[0] = note;
  p->accents[0] = accent;
  // update the voice counter
  p->n = m;
}

void SynthVoice::mva_note_off(mva_data *p, uint8_t note)
{
  uint8_t s = 0;

  // find if the note is actually in the buffer
  uint8_t m = p->n;
  uint8_t i = m;
  while (i) // count backwards (oldest notes first)
  {
    --i;
    if (note == p->notes[i] )
    {
      // found it!
      if (i < (p->n - 1)) // don't shift if this was the last note..
      {
        // remove it now.. just shift everything after it
        s = i;
        while (i < m)
        {
          ++s;
          p->notes[i] = p->notes[s];
          p->accents[i] = p->accents[s];
          i = s;
        }
      }
      // update the voice counter
      if (m > 0) {
        p->n = m - 1;
      }
      break;
    }
  }
}

void SynthVoice::mva_reset(mva_data *p) {
  p->n = 0;
}

void  SynthVoice::note_on(uint8_t midiNote, bool slide, bool accent) {
  _accent = accent;
  _slide = slide || _portamento;

#ifdef DEBUG_SYNTH
  DEBF("synth: note: %d, slide: %d, accent: %d\r\n", midiNote, slide, accent);
#endif

  _targetStep = Tables::midi_tbl_steps[midiNote];
  _effectiveStep = _targetStep * _tuning * _pitchbend;
  if (mva1.n == 1) {
    if (_accent) {
      _accentation = _accentLevel; 
      AmpEnv.setReleaseTimeMs(_ampReleaseMs * 50.0f);
      FltEnv.setDecayTimeMs(_filterAccentDecayMs );
      FltEnv.setAttackTimeMs(_filterAccentAttackMs );
    } else {
      _accentation = 0.0f;
      AmpEnv.setReleaseTimeMs( _ampReleaseMs );
      FltEnv.setDecayTimeMs(_filterDecayMs );
      FltEnv.setAttackTimeMs(_filterAttackMs );
    }
  }
  AmpEnv.setAttackTimeMs(_ampAttackMs);
  AmpEnv.setDecayTimeMs(_ampDecayMs);
  if (_slide) {
    _deltaStep = (_effectiveStep - _currentStep) * (1000.0f * DIV_SAMPLE_RATE / _slideMs );
  } else {
    _currentStep = _effectiveStep;
    _deltaStep = 0.0f ;
    _phaze = 0.0f;
    AmpEnv.retrigger(Adsr::END_NOW);
    FltEnv.retrigger(false);    
  }
  _k_acc = (1.0f + 0.6f * _accentation);
 // DEBF ("ampRelease %f \t ampDecay %f \t \r\n", _ampReleaseMs, _ampDecayMs );
}

void  SynthVoice::note_off() {
  AmpEnv.end(Adsr::END_REGULAR);
 // FltEnv.end(false);
}
