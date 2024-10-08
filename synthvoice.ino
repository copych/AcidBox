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
  _pan = 0.5;

  // parameters of envelopes
  _ampEnvPosition = 0.0;
  _filterEnvPosition = 0.0;
  _ampAttackMs = 3.0;
  _ampDecayMs = 300.0;
  _ampReleaseMs = 30.0;
  _ampEnvAttackStep = 15.0;
  _ampEnvDecayStep = 1.0;
  _ampEnvReleaseStep = 15.0;
  _filterAttackMs = 5.0;
  _filterDecayMs = 200.0;
  _filterEnvAttackStep = 15.0;
  _filterEnvDecayStep = 1.0;

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
  ampDeclicker.setGain( amp2dB(sqrt(0.5f)) );
  ampDeclicker.setFrequency(200.0f);
  filtDeclicker.setMode(BiquadFilter::LOWPASS12);
  filtDeclicker.setGain( amp2dB(sqrt(0.5f)) );
  filtDeclicker.setFrequency(200.0f);
  notch.setMode(BiquadFilter::BANDREJECT);
  notch.setFrequency(7.5164f);
  notch.setBandwidth(4.7f);
}


inline float SynthVoice::getSample() {
  float samp = 0.0f, filtEnv = 0.0f, ampEnv = 0.0f, final_cut = 0.0f;
    filtEnv = GetFilterEnv();
    ampEnv = GetAmpEnv();
    if (_eAmpEnvState != ENV_IDLE) {
      // samp = (float)((1.0f - _waveMix) * lookupTable(*(tables[_waveBase]), _phaze)) + (float)(_waveMix * lookupTable(*(tables[_waveBase+1]), _phaze)) ; // lookup and blend waveforms
      samp = (float)((1.0f - _waveMix) * lookupTable(exp_square_tbl, _phaze)) + (float)(_waveMix * lookupTable(saw_tbl, _phaze)) ; // lookup and blend waveforms
    } else {
      samp = 0.0f;
    }
    final_cut = (float)_filter_freq * ( (float)_envMod * ((float)filtEnv - 0.2f) + 1.3f * (float)_accentation + 1.0f );
    final_cut = filtDeclicker.getSample( final_cut );
    Filter.SetCutoff( final_cut );

    samp = highpass1.getSample(samp);         // pre-filter highpass, following open303
    
    samp = allpass.getSample(samp);           // phase correction, following open303
   
    samp = Filter.Process(samp);              // main filter
    
    samp = highpass2.getSample(samp);         // post-filtering, following open303
    
    samp = notch.getSample(samp);             // post-filtering, following open303
    
    samp = Drive.Process(samp);               // overdrive
    
    samp = Distortion.Process(samp);             // distortion
    
    samp *= ampEnv;                           // amp envelope


    _compens = _volume * 8.0f  * _fx_compens ; // * _flt_compens;
    
    _compens = ampDeclicker.getSample(_compens);
    
    samp *=  _compens;

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
    /* 
     *  // this is more accurate classical approach, but it gives quite early audible aliasing when not using band-limited samples
    if ( _phaze >= TABLE_SIZE) {
       _phaze -= TABLE_SIZE ;
    */
   
     
    // this is less accurate in terms of pitch, especially at higher notes, but at this price you have quite no aliasing, as phase reset produces no moire
    if ( _phaze >= TABLE_SIZE) {
      if (_wave_cnt == 3) { // we drop the phase every 4 periods
        _wave_cnt = 0; 
        _phaze = 0.0f; // we reset the phase, so aliasing will be present in the form of lower octave tones which is less annoying
      } else {
        _wave_cnt++;
        _phaze -= TABLE_SIZE ;
      }
       
    }
    
    /*
    // this is less accurate in terms of pitch, especially at higher notes, but at this price you have quite no aliasing, as phase reset produces no moire
    if ( _phaze >= TABLE_SIZE) {
      _phaze = 2.0f * (float)( (int)(0.5f * (_phaze - (float)TABLE_SIZE)) ); 
      DEBF("%0.5f\r\n", _phaze);
    }*/

    
    //synth_buf[_index][i] = fast_shape(samp); // mono limitter
    return  samp;
  
}


inline void SynthVoice::SetCutoff(float lvl)  {
  _cutoff = lvl;
  _filter_freq = knobMap( lvl, MIN_CUTOFF_FREQ, MAX_CUTOFF_FREQ);
#ifdef DEBUG_SYNTH
  DEBF("Synth %d cutoff=%0.3f freq=%0.3f\r\n" , _index, _cutoff, _filter_freq);
#endif
}

inline void SynthVoice::PitchBend(int number) {
  //
  DEBUG(number);
}

inline void SynthVoice::ParseCC(uint8_t cc_number , uint8_t cc_value) {
  float tmp = 0.0f;
  switch (cc_number) {

    case CC_303_PORTATIME:
      _slideMs = (float)cc_value;
      break;
    case CC_303_VOLUME:
      _volume = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_PAN:
      _pan = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_PORTAMENTO:
      _portamento = (cc_value >= 64);
      break;
    case CC_303_WAVEFORM:
      /*
       // actually we can gradually switch between several waveforms, basing on the CC value, blending neighbour two waveforms
        _waveBase = (uint8_t)(((float)cc_value * 2.99999f * MIDI_NORM)) ; // 0, 1, 2 range
        DEBF("base %d\r\n", _waveBase );
        _waveMix = ((float)cc_value - (float)(_waveBase*42.33333f)) * MIDI_NORM * 3.0f;
        DEBF("mix %0.5f\r\n", _waveMix );*/
      _waveMix = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_RESO:
      _reso = cc_value * MIDI_NORM ;
      _flt_compens = one_div( bilinearLookup(norm1_tbl, _cutoff * 127.0f, cc_value ));
      SetReso(_reso);
      break;
    case CC_303_DECAY: // Env release
      tmp = (float)cc_value * MIDI_NORM;
      _filterDecayMs = knobMap(tmp, 15.0f, 5000.0f);
      _ampDecayMs = knobMap(tmp, 15.0f, 7500.0f);
      break;
    case CC_303_ATTACK: // Env attack
      tmp = (float)cc_value * MIDI_NORM;
      _filterAttackMs = knobMap(tmp, 3.0f, 500.0f);
      _ampAttackMs =  knobMap(tmp, 3.0f, 700.0f);
      break;
    case CC_303_CUTOFF:
      _cutoff = (float)cc_value * MIDI_NORM;
      _flt_compens = one_div( bilinearLookup(norm1_tbl, cc_value, _reso * 127.0f));
      SetCutoff(_cutoff);
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
      _fx_compens = one_div( bilinearLookup(norm2_tbl, _drive * 127.0f,  cc_value));
      SetDistortionLevel(_gain);
      break;
    case CC_303_OVERDRIVE:
      _drive = (float)cc_value * MIDI_NORM ;
      _fx_compens = one_div( bilinearLookup(norm2_tbl, cc_value, _gain * 127.0f));
      SetOverdriveLevel(_drive);
      break;
    case CC_303_SATURATOR:
      _saturator = (float)cc_value * MIDI_NORM;
      Filter.SetDrive(_saturator);
      break;
  }
}

float SynthVoice::GetAmpEnv() {
  switch (_eAmpEnvState) {
    case ENV_INIT:
      _k_acc = (1.0f + 0.3f * _accentation);
      _ampEnvPosition = 0.0f;
      _ampEnvAttackStep = _msToSteps * one_div( _ampAttackMs + 0.0001f);
      _ampEnvDecayStep = _msToSteps * one_div(_ampDecayMs + 0.0001f);
      _ampEnvReleaseStep = _msToSteps * one_div(_ampReleaseMs + 0.0001f);
      if (_accent) {
        _ampEnvDecayStep *= 3.0f;
        _ampEnvReleaseStep = _msToSteps * one_div(_ampAccentReleaseMs + 0.0001f);
      }
      _eAmpEnvState = ENV_ATTACK;
      _ampEnvVal = (-exp_tbl[ 0 ] + 1.0f) * 0.5f;
      break;
    case ENV_ATTACK:
      _ampEnvPosition += _ampEnvAttackStep;
      if (_ampEnvPosition >= TABLE_SIZE) {
        _eAmpEnvState = ENV_DECAY;
        _ampEnvPosition = 0;
        _ampEnvVal = (-exp_tbl[ TABLE_SIZE - 1 ] + 1.0f) * 0.5f * _k_acc;
      } else {
        _ampEnvVal = (-lookupTable(exp_tbl, _ampEnvPosition ) + 1.0f) * 0.5f * _k_acc;
        if (_pass_val > _ampEnvVal) _ampEnvVal = _pass_val;
      }
      _pass_val = _ampEnvVal;
      break;
    case ENV_DECAY:
      _ampEnvPosition += _ampEnvDecayStep;
      if (_ampEnvPosition >= TABLE_SIZE) {
        _eAmpEnvState = ENV_SUSTAIN;
        _ampEnvPosition = 0;
        _ampEnvVal = _sust_level;
      } else {
        _ampEnvVal = _sust_level + (1.0f - _sust_level) * (lookupTable(exp_tbl, _ampEnvPosition) + 1.0f) * 0.5f * _k_acc;
      }
      _pass_val = _ampEnvVal;
      break;
    case ENV_SUSTAIN:
      _ampEnvVal = _sust_level; //  asuming sustain to be endless
      _pass_val = _ampEnvVal;
      _ampEnvPosition = 0;
      break;
    case ENV_RELEASE:
      if (_ampEnvPosition >= TABLE_SIZE) {
        _eAmpEnvState = ENV_IDLE;
        _ampEnvPosition = 0;
        _ampEnvVal = 0.0f;
      } else {
        if (_ampEnvPosition <= _ampEnvReleaseStep) _release_lvl = _pass_val;
        _ampEnvVal = _release_lvl * (lookupTable(exp_tbl, _ampEnvPosition) + 1.0f) * 0.5f * _k_acc;
      }
      _ampEnvPosition += _ampEnvReleaseStep;
      _pass_val = _ampEnvVal;
      break;
    case ENV_IDLE:
      _ampEnvVal = 0.0f;
      break;
    default:
      _ampEnvVal = 0.0f;
  }
  if (_accent) {

  }
  return _ampEnvVal;
}

inline float SynthVoice::GetFilterEnv() {
  switch (_eFilterEnvState) {
    case ENV_INIT:
      //   k_acc = (1.0f + 0.45f * _accentation);
      _offset = max(_filterEnvVal, _offset);
      _filterEnvPosition = 0.0f;
      _filterEnvAttackStep = _msToSteps * one_div(_filterAttackMs + 0.0001f);
      _filterEnvDecayStep = _msToSteps * one_div(_filterDecayMs + 0.0001f);
      if (_accent) {
        _filterEnvAttackStep *= 1.4f;
        _filterEnvDecayStep *= 5.0f;
      }
      _eFilterEnvState = ENV_ATTACK;
      _filterEnvVal = (-exp_tbl[ 0 ] + 1.0f) * 0.5f ;
      break;
    case ENV_ATTACK:
      if (_filterEnvPosition >= (float)TABLE_SIZE) {
        _eFilterEnvState = ENV_DECAY;
        _filterEnvPosition = 0.0f;
        _filterEnvVal = (-exp_tbl[ TABLE_SIZE - 1 ] + 1.0f) * 0.5f ;
      } else {
        _filterEnvVal = (-lookupTable(exp_tbl, _filterEnvPosition) + 1.0f) * 0.5f  ;
      }
      _filterEnvPosition += _filterEnvAttackStep;
      break;
    case ENV_DECAY:
      if (_filterEnvPosition >= (float)TABLE_SIZE) {
        _eFilterEnvState = ENV_IDLE; // Attack-Decay-0 envelope (?)
        _filterEnvPosition = 0.0f;
        _filterEnvVal = 0.0f;
      } else {
        _filterEnvVal =  (lookupTable(exp_tbl, _filterEnvPosition) + 1.0f) * 0.5f  ;
      }
      _filterEnvPosition += _filterEnvDecayStep;
      _offset *= _offset_leak;
      break;
    case ENV_IDLE:
      _filterEnvVal =  0.0f;
      _offset *= _offset_leak;
      break;
    default:
      _filterEnvVal =  0.0f;
  }
  _filterEnvVal += _offset;
  return _filterEnvVal ;
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
    _phaze = 0.0f;

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
