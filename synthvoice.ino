#include "synthvoice.h"


void SynthVoice::Init() {
  _envMod = 0.5f;
  _accentLevel = 0.5f;
  _cutoff = 0.2f; // 0..1 normalized freq range. Keep in mind that EnvMod set to max practically floats this range
  _filter_freq = linToExp(_cutoff, 0.0f, 1.0f, MIN_CUTOFF_FREQ, MAX_CUTOFF_FREQ);
  _reso = 0.4f;
  _gain = 0.0f; // values >1 will distort sound
  _drive = 0.0f;
  _eAmpEnvState = ENV_IDLE;
  _eFilterEnvState = ENV_IDLE;
  //  midiNotes[0] = -1;
  //  midiNotes[1] = -1;
  _midiNote = 69;
  _currentStep = 1.0f;
  _targetStep = 1.0f;
  _deltaStep = 0.0f;
  _slideMs = 60.0f;
  _phaze = 0.0f;
  mva1.n = 0 ;

  // parameters of envelopes
  _ampEnvPosition = 0.0;
  _filterEnvPosition = 0.0;
  _ampAttackMs = 3.0;
  _ampDecayMs = 300.0;
  _ampReleaseMs = 3.0;
  _ampEnvAttackStep = 15.0;
  _ampEnvDecayStep = 1.0;
  _ampEnvReleaseStep = 15.0;
  _filterAttackMs = 5.0;
  _filterDecayMs = 200.0;
  _filterEnvAttackStep = 15.0;
  _filterEnvDecayStep = 1.0;

  Wfolder.Init();
  Drive.Init();

  Filter.Init((float)SAMPLE_RATE);
  Filter.SetMode(TeeBeeFilter::LP_18);
  highpass1.setMode(OnePoleFilter::HIGHPASS);
  highpass1.setCutoff(44.486f);
  highpass2.setMode(OnePoleFilter::HIGHPASS);
  highpass2.setCutoff(24.167f);
  allpass.setMode(OnePoleFilter::ALLPASS);
  allpass.setCutoff(14.008f);
}



inline void SynthVoice::Generate() {
  float samp = 0.0f, filtEnv = 0.0f, ampEnv = 0.0f, final_cut = 0.0f;
  for (uint16_t i = 0; i < DMA_BUF_LEN; ++i) {
    prescaler += (1 - _index);
    filtEnv = GetFilterEnv();
    ampEnv = GetAmpEnv();
    if (_eAmpEnvState != ENV_IDLE) {
      // samp = (float)((1.0f - _waveMix) * lookupTable(*(tables[_waveBase]), _phaze)) + (float)(_waveMix * lookupTable(*(tables[_waveBase+1]), _phaze)) ; // lookup and blend waveforms
      samp = (float)((1.0f - _waveMix) * lookupTable(exp_square_tbl, _phaze)) + (float)(_waveMix * lookupTable(exp_tbl, _phaze)) ; // lookup and blend waveforms
    } else {
      samp = 0.0f;
    }
    //  if (i % 4 == 0) {
    final_cut = (float)_filter_freq * ( (float)_envMod * ((float)filtEnv - 0.2f) + 0.3f * (float)_accentation + 1.0f );
    final_cut = filtDeclicker.Process( final_cut);
    Filter.SetCutoff( final_cut );
    //    }

    samp = highpass1.getSample(samp);        // pre-filter highpass
    samp = Filter.Process(samp);
    samp = allpass.getSample(samp);
    samp = highpass2.getSample(samp);

    /*
        if ( i % DMA_BUF_LEN == 0 && _index == 0) {
          avgmax = avgmax*0.999 - (avgmax - avgmid) * 0.001f ;
          avgmin = avgmin*0.999 + (avgmid - avgmin) * 0.001f ;
          if (samp > avgmax) avgmax = samp ;
          else if (samp < avgmin) avgmin = samp ;
      //    avgmid = 0.5f * (avgmax+avgmin) * 0.001f + 0.999f * avgmid;
       //   DEBF( " %0.3f  %0.3f  %0.3f\r\n", avgmin, avgmid, avgmax );
          DEBF("%0.3f\r\n", avgmax-avgmin);
      //  samp -= avgmid;
        }

    */
    ampEnv = ampDeclicker.Process(ampEnv);
    samp *= ampEnv;

    samp = Drive.Process(samp);
    samp = Wfolder.Process(samp);

    samp *=  volume;



    //   if ( prescaler % DMA_BUF_LEN == 0 && _index == 0 ) DEBF( "off(Hz) %0.3f\r\n", _offset );

    if ((_slide || _portamento) && _deltaStep != 0.0f) {     // portamento / slide processing
      if (fabs(_targetStep - _currentStep) >= fabs(_deltaStep)) {
        _currentStep += _deltaStep;
      } else {
        _currentStep = _targetStep;
        _deltaStep = 0.0f;
      }
    }

    // Increment and wrap phase
    _phaze += _currentStep;
    if ( _phaze >= WAVE_SIZE) {
      // _phaze -= WAVE_SIZE ;
      _phaze = 0.0f;
    }

    //synth_buf[_index][i] = fast_tanh(samp); // mono
    //synth_buf[_index][i] = ampDeclicker.Process(samp);
    synth_buf[_index][i] = samp;

  }
  //DEBF("synt%d = %0.5f\r\n", _index , samp);
}


inline void SynthVoice::ParseCC(uint8_t cc_number , uint8_t cc_value) {
  switch (cc_number) {

    case CC_303_PORTATIME:
      _slideMs = (float)cc_value;
      break;
    case CC_303_VOLUME:
      volume = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_PAN:
      pan = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_PORTAMENTO:
      _portamento = (cc_value >= 64);
      break;
    case CC_303_WAVEFORM:
      /*
        _waveBase = (uint8_t)(((float)cc_value * 2.99999f * MIDI_NORM)) ; // 0, 1, 2 range
        DEBF("base %d\r\n", _waveBase );
        _waveMix = ((float)cc_value - (float)(_waveBase*42.33333f)) * MIDI_NORM * 3.0f;
        DEBF("mix %0.5f\r\n", _waveMix );*/
      _waveMix = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_RESO:
      _reso = cc_value * MIDI_NORM ;
      Filter.SetResonance(_reso);
      break;
    case CC_303_DECAY: // Env release
      _filterDecayMs = 15.0f + (float)cc_value * MIDI_NORM * 5000.0f ;
      _ampDecayMs = 15.0f + (float)cc_value * MIDI_NORM * 7500.0f;
      break;
    case CC_303_ATTACK: // Env attack
      _filterAttackMs = 3.0f + (float)cc_value * MIDI_NORM * 500.0f ;
      _ampAttackMs =  3.0f + (float)cc_value * MIDI_NORM * 700.0f;
      break;
    case CC_303_CUTOFF:
      _cutoff = (float)cc_value * MIDI_NORM;
      _filter_freq = linToExp(_cutoff, 0.0f, 1.0f, MIN_CUTOFF_FREQ, MAX_CUTOFF_FREQ);
      break;
    case CC_303_DELAY_SEND:
      _sendDelay = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_REVERB_SEND:
      _sendReverb = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_ENVMOD_LVL:
      _envMod = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_ACCENT_LVL:
      _accentLevel = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_DISTORTION:
      _gain = (float)cc_value * MIDI_NORM ;
      Wfolder.SetDrive(_gain );
      break;
    case CC_303_OVERDRIVE:
      _drive = (float)cc_value * MIDI_NORM ;
      Drive.SetDrive(_drive );
      break;
    case CC_303_SATURATOR:
      _saturator = (float)cc_value * MIDI_NORM;
      Filter.SetDrive(_saturator);
      break;
  }
}

float SynthVoice::GetAmpEnv() {
  const static float sust_level = 0.35f;
  const static float k_sust = 1.0f - sust_level;
  static float ret_val = 0.0f;
  static float pass_val = 0.0f, release_lvl = 0.0f;
  static float k_acc = 1.0f;
  switch (_eAmpEnvState) {
    case ENV_INIT:
      k_acc = (1.0f + 0.3f * _accentation);
      _ampEnvPosition = 0.0f;
      _ampEnvAttackStep = _msToSteps * one_div( _ampAttackMs + 0.0001f);
      _ampEnvDecayStep = _msToSteps * one_div(_ampDecayMs + 0.0001f);
      _ampEnvReleaseStep = _msToSteps * one_div(_ampReleaseMs + 0.0001f);
      if (_accent) {
        _ampEnvDecayStep *= 3.0f;
        _ampEnvReleaseStep = _msToSteps * one_div(50.0f + 0.0001f);
      }
      _eAmpEnvState = ENV_ATTACK;
      ret_val = (-exp_tbl[ 0 ] + 1.0f) * 0.5f;
      break;
    case ENV_ATTACK:
      _ampEnvPosition += _ampEnvAttackStep;
      if (_ampEnvPosition >= WAVE_SIZE) {
        _eAmpEnvState = ENV_DECAY;
        _ampEnvPosition = 0;
        ret_val = (-exp_tbl[ WAVE_SIZE - 1 ] + 1.0f) * 0.5f * k_acc;
      } else {
        ret_val = (-lookupTable(exp_tbl, _ampEnvPosition ) + 1.0f) * 0.5f * k_acc;
        if (pass_val > ret_val) ret_val = pass_val;
      }
      pass_val = ret_val;
      break;
    case ENV_DECAY:
      _ampEnvPosition += _ampEnvDecayStep;
      if (_ampEnvPosition >= WAVE_SIZE) {
        _eAmpEnvState = ENV_SUSTAIN;
        _ampEnvPosition = 0;
        ret_val = sust_level;
      } else {
        ret_val = sust_level + k_sust * (lookupTable(exp_tbl, _ampEnvPosition) + 1.0f) * 0.5f * k_acc;
      }
      pass_val = ret_val;
      break;
    case ENV_SUSTAIN:
      ret_val = sust_level; //  asuming sustain to be endless
      pass_val = ret_val;
      _ampEnvPosition = 0;
      break;
    case ENV_RELEASE:
      if (_ampEnvPosition >= WAVE_SIZE) {
        _eAmpEnvState = ENV_IDLE;
        _ampEnvPosition = 0;
        ret_val = 0.0f;
      } else {
        if (_ampEnvPosition <= _ampEnvReleaseStep) release_lvl = pass_val;
        ret_val = release_lvl * (lookupTable(exp_tbl, _ampEnvPosition) + 1.0f) * 0.5f * k_acc;
      }
      _ampEnvPosition += _ampEnvReleaseStep;
      pass_val = ret_val;
      break;
    case ENV_IDLE:
      ret_val = 0.0f;
      break;
    default:
      ret_val = 0.0f;
  }
  if (_accent) {

  }
  return ret_val;
}

inline float SynthVoice::GetFilterEnv() {
  static volatile float ret_val = 0.0f;
  switch (_eFilterEnvState) {
    case ENV_INIT:
      //   k_acc = (1.0f + 0.45f * _accentation);
      _offset = max(ret_val, _offset);
      _filterEnvPosition = 0.0f;
      _filterEnvAttackStep = _msToSteps * one_div(_filterAttackMs + 0.0001f);
      _filterEnvDecayStep = _msToSteps * one_div(_filterDecayMs + 0.0001f);
      if (_accent) {
        _filterEnvAttackStep *= 1.4f;
        _filterEnvDecayStep *= 5.0f;
      }
      _eFilterEnvState = ENV_ATTACK;
      ret_val = (-exp_tbl[ 0 ] + 1.0f) * 0.5f ;
      break;
    case ENV_ATTACK:
      if (_filterEnvPosition >= (float)WAVE_SIZE) {
        _eFilterEnvState = ENV_DECAY;
        _filterEnvPosition = 0.0f;
        ret_val = (-exp_tbl[ WAVE_SIZE - 1 ] + 1.0f) * 0.5f ;
      } else {
        ret_val = (-lookupTable(exp_tbl, _filterEnvPosition) + 1.0f) * 0.5f  ;
      }
      _filterEnvPosition += _filterEnvAttackStep;
      break;
    case ENV_DECAY:
      if (_filterEnvPosition >= (float)WAVE_SIZE) {
        _eFilterEnvState = ENV_IDLE; // Attack-Decay-0 envelope (?)
        _filterEnvPosition = 0.0f;
        ret_val = 0.0f;
      } else {
        ret_val =  (lookupTable(exp_tbl, _filterEnvPosition) + 1.0f) * 0.5f  ;
      }
      _filterEnvPosition += _filterEnvDecayStep;
      _offset *= _offset_leak;
      break;
    case ENV_IDLE:
      ret_val =  0.0f;
      _offset *= _offset_leak;
      break;
    default:
      ret_val =  0.0f;
  }
  ret_val += _offset;
  return ret_val ;
}

// calcEnvModScalerAndOffset() taken from open303 code
inline void SynthVoice::calcEnvModScalerAndOffset() {
  // define some constants that arise from the measurements:
  const float c0   = 313.0f;  // lowest nominal cutoff
  const float c1   = 2394.0f;  // highest nominal cutoff
  const float oF   = 0.048292930943553f;       // factor in line equation for offset
  const float oC   = 0.294391201442418f;       // constant in line equation for offset
  const float sLoF = 3.773996325111173f;       // factor in line eq. for scaler at low cutoff
  const float sLoC = 0.736965594166206f;       // constant in line eq. for scaler at low cutoff
  const float sHiF = 4.194548788411135f;       // factor in line eq. for scaler at high cutoff
  const float sHiC = 0.864344900642434f;       // constant in line eq. for scaler at high cutoff

  // do the calculation of the scaler and offset:
  float e   = _envMod;
  //  float c   = expToLin(_filter_freq, c0,   c1,   0.0, 1.0);
  float c   = _cutoff;
  float sLo = sLoF * e + sLoC;
  float sHi = sHiF * e + sHiC;
  _envScaler  = (1 - c) * sLo + c * sHi;
  _envOffset  =  oF * c + oC;
}

// The following code initially written by Anton Savov,
// is taken from http://antonsavov.net/cms/projects/303andmidi.html
// Monophonic Voice Allocator (with Accent, suitable for the 303)
// "Newest" note-priority rule
// Modified version, allows multiple Notes with the same pitch

inline void SynthVoice::on_midi_noteON(uint8_t note, uint8_t velocity)
{
  mva_note_on(&mva1, note, (velocity >= 80));

  bool slide = (mva1.n > 1);
  bool accent = (mva1.accents[0]);
  note = mva1.notes[0] ;
  note_on(note, slide, accent);
}

inline void SynthVoice::on_midi_noteOFF(uint8_t note, uint8_t velocity)
{
  if (mva1.n == 0) {
    return;
  }
  uint8_t tmp_note = mva1.notes[0];
  uint8_t tmp_accent = mva1.accents[0];
  mva_note_off(&mva1, note);

  if (mva1.n > 0)
  {
    if (mva1.notes[0] != tmp_note)
    {
      bool accent = (mva1.accents[0] );
      bool slide = 1;
      note = mva1.notes[0];

      note_on(note, slide, accent);
    }
  }
  else {
    note_off();
  }
}


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

void SynthVoice::mva_reset(mva_data *p)
{
  p->n = 0;
}

void  SynthVoice::note_on(uint8_t midiNote, bool slide, bool accent)
{
  _accent = accent;
  _slide = slide || _portamento;
  _targetStep = midi_tbl_steps[midiNote];
  if (_slide) {
    _deltaStep = (_targetStep - _currentStep) * (1000.0f * DIV_SAMPLE_RATE / _slideMs );
  } else {
    _currentStep = _targetStep;
    _deltaStep = 0.0f ;
    _eAmpEnvState = ENV_INIT;
    _eFilterEnvState = ENV_INIT;

  }
  if (mva1.n == 1) {
    if (_accent) _accentation = _accentLevel; else _accentation = 0.0f;
  }
}

void  SynthVoice::note_off()
{
  _eAmpEnvState = ENV_RELEASE;
  _ampEnvPosition = 0;
}
