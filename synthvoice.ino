#include "synthvoice.h"


void SynthVoice::Init() {
  _envMod = 0.0f;   
  _accentLevel = 0.0f; 
  _cutoff = 0.2f; // 0..1 normalized freq range. Keep in mind that EnvMod set to max practically doubles this range
  _reso = 0.4f;
  _gain = 1.0; // values >1 will distort sound
  _eAmpEnvState = ENV_IDLE;
  _eFilterEnvState = ENV_IDLE;
//  midiNotes[0] = -1;
//  midiNotes[1] = -1;
  _midiNote = 69;
  _currentStep = 1.0f;
  _targetStep = 1.0f;
  _deltaStep = 1.0f;
  _slideMs = 60.0f;
  _phaze = 0.0f;
  mva1.n = 0 ;
  
  // MEG and FEG params
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
   
  Filter.Init((float)SAMPLE_RATE);  
  if (_index==0) {
    Wfolder.Init();
  } else {
    Drive.Init();
  }
}

inline void SynthVoice::Generate() {
  float samp = 0.0f, filtEnv = 0.0f, ampEnv = 0.0f, final_cut = 0.0f;
  static float meter1=0.0f, meter=0.0f;
  static float avgmin=0.0f, avgmax=0.0f, avgmid=0.0f;
  for (int i = 0; i < DMA_BUF_LEN; ++i) {
  prescaler++;
  filtEnv = GetFilterEnv();
  ampEnv = GetAmpEnv();
//  filtEnv = filtDeclicker.Process(filtEnv);
//  ampEnv = ampDeclicker.Process(ampEnv);
    if (_eAmpEnvState != ENV_IDLE) {
      //samp = (((1.0f - _waveMix) * (square_2048[ (uint16_t)(_phaze) ] ) ) + ( _waveMix * exp_2048[ (uint16_t)(_phaze) ] )); // lookup and mix waveforms
      samp = (float)(1.0f - _waveMix) * (float)lookupTable(square_2048,_phaze) + (float)_waveMix * (float)lookupTable(exp_2048,_phaze) ; // lookup and mix waveforms
    } else {
      samp = 0.0f;
    }
    if (i % 2 == 0) {
      final_cut = (float)MIN_CUTOFF_FREQ + (float)(MAX_CUTOFF_FREQ - MIN_CUTOFF_FREQ) * (_cutoff * (1.0f - 0.2f * _envMod) + (_envMod * (filtEnv - 0.15f)));
      Filter.SetCutoff( final_cut );
    }
    
 //   if ( prescaler % DMA_BUF_LEN == 0) meter = meter * 0.95f + abs( samp); 
    samp = Filter.Process(samp); 
 /*  
    if ( prescaler % DMA_BUF_LEN == 0 && _index == 0) {
      avgmax = avgmax - (avgmax - avgmid) * 0.001f ;
      avgmin = avgmin + (avgmid - avgmin) * 0.001f ;
      if (samp > avgmax) avgmax = samp ;
      if (samp < avgmin) avgmin = samp ;
      avgmid = samp * 0.001f + 0.999f * avgmid;
      DEBF( " %0.5f  %0.5f  %0.5f\r\n", avgmin, avgmid, avgmax );
    }
  */
   // if ( prescaler % DMA_BUF_LEN == 0 && _index == 0 ) DEBF("%0.3f\r\n", samp); 
 //   if ( prescaler % DMA_BUF_LEN == 0 && _index == 0 ) DEBF( "sat %0.5f pre %0.5f post %0.5f\r\n", _saturator, meter, meter1 );
    samp *= ampEnv ;

    switch (_index) {
      case 0:
        samp = Wfolder.Process(samp);
        break;
      case 1:
      default:
        samp = Drive.Process(samp);
        break;
    }
    
    samp *=  volume;
//   if ( prescaler % DMA_BUF_LEN == 0 && _index == 0 ) DEBF( "f(Hz) %0.0f\r\n", final_cut );
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
      _phaze -= WAVE_SIZE ;
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
      _waveMix = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_RESO:
      _reso = cc_value * MIDI_NORM ;
      Filter.SetResonance(_reso);
      //Filter.setResonance(_reso);
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
      if (_index == 0 ) {
        Wfolder.SetDrive(_gain );
      } else {
        Drive.SetDrive(_gain ); 
      }
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
  k_acc = (1.0f + 0.3f * _accentation);
  switch (_eAmpEnvState) {
    case ENV_INIT:
      _ampEnvPosition = 0.0f;
      _ampEnvAttackStep = _msToSteps * one_div( _ampAttackMs+0.0001f);
      _ampEnvDecayStep = _msToSteps * one_div(_ampDecayMs+0.0001f);
      _ampEnvReleaseStep = _msToSteps * one_div(_ampReleaseMs+0.0001f);
      if (_accent) {
        _ampEnvDecayStep *= 3.0f;
        _ampEnvReleaseStep = _msToSteps * one_div(50.0f +0.0001f);
      }
      _eAmpEnvState = ENV_ATTACK;
      ret_val = (-exp_2048[ 0 ] + 1.0f) * 0.5f;
      break;
    case ENV_ATTACK:
      _ampEnvPosition += _ampEnvAttackStep;
      if (_ampEnvPosition >= WAVE_SIZE) {
        _eAmpEnvState = ENV_DECAY;
        _ampEnvPosition = 0;
        ret_val = (-exp_2048[ WAVE_SIZE-1 ] + 1.0f) * 0.5f * k_acc;
      } else {
        ret_val = (-lookupTable(exp_2048, _ampEnvPosition ) + 1.0f) * 0.5f * k_acc;
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
        ret_val = sust_level + k_sust * (lookupTable(exp_2048, _ampEnvPosition) + 1.0f) * 0.5f * k_acc;
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
        ret_val = release_lvl * (lookupTable(exp_2048, _ampEnvPosition) + 1.0f) * 0.5f * k_acc;
      }
      _ampEnvPosition += _ampEnvReleaseStep;
      pass_val=ret_val;
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
  static volatile float k_acc = 1.0f;
  k_acc = (1.0f + 0.3f * _accentation);
  switch(_eFilterEnvState) {
    case ENV_INIT:
      _offset = max(ret_val, _offset);
      _filterEnvPosition = 0.0f;
      _filterEnvAttackStep = _msToSteps * one_div(_filterAttackMs+0.0001f);
      _filterEnvDecayStep = _msToSteps * one_div(_filterDecayMs+0.0001f);
      if (_accent) {
        _filterEnvAttackStep *= 1.4f;
        _filterEnvDecayStep *= 5.0f;
      }
      _eFilterEnvState = ENV_ATTACK;
      ret_val = (-exp_2048[ 0 ] + 1.0f) * 0.5f * k_acc;
      break;
    case ENV_ATTACK:
      if (_filterEnvPosition >= (float)WAVE_SIZE) {
        _eFilterEnvState = ENV_DECAY;
        _filterEnvPosition = 0.0f;
        ret_val = (-exp_2048[ WAVE_SIZE-1 ] + 1.0f) * 0.5f * k_acc;
      } else {
        ret_val = (-lookupTable(exp_2048, _filterEnvPosition) + 1.0f) * 0.5f * k_acc ;
      }
      _filterEnvPosition += _filterEnvAttackStep;
      break;
    case ENV_DECAY:
      if (_filterEnvPosition >= (float)WAVE_SIZE) {
        _eFilterEnvState = ENV_IDLE; // Attack-Decay-0 envelope (?)
        _filterEnvPosition = 0.0f;
        ret_val = 0.0f;
      } else {
        ret_val =  (lookupTable(exp_2048, _filterEnvPosition) + 1.0f) * 0.5f * k_acc ;
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


// The following code initially written by Anton Savov,
// is taken from http://antonsavov.net/cms/projects/303andmidi.html
// Monophonic Voice Allocator (with Accent, suitable for the 303)
// "Newest" note-priority rule
// Modified version, allows multiple Notes with the same pitch

void SynthVoice::mva_note_on(mva_data *p, uint8_t note, uint8_t accent)
{
    if (accent) { accent = 0x80; }
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
        p->buf[i] = p->buf[s];
        i = s;
    }
    // put the new note first
    p->buf[0] = note | accent;
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
        if (note == (p->buf[i] & 0x7F))
        {
            // found it!
            if (i < (p->n - 1)) // don't shift if this was the last note..
            {
                // remove it now.. just shift everything after it
                s = i;
                while (i < m)
                {
                    ++s;
                    p->buf[i] = p->buf[s];
                    i = s;
                }
            }
            // update the voice counter
            if (m > 0) { p->n = m - 1; }
            break;
        }
    }
}

void SynthVoice::mva_reset(mva_data *p)
{
    p->n = 0;
}

inline void SynthVoice::on_midi_noteON(uint8_t note, uint8_t velocity)
{
    mva_note_on(&mva1, note, (velocity >= 100));

    bool slide = (mva1.n > 1);
    bool accent = (mva1.buf[0] >> 7); // top bit
    note = mva1.buf[0] & 0x7F;

    note_on(note, slide, accent);
}

inline void SynthVoice::on_midi_noteOFF(uint8_t note, uint8_t velocity)
{
    if (mva1.n == 0) { return; }
    uint8_t tmp = mva1.buf[0];
    mva_note_off(&mva1, note);

    if (mva1.n > 0)
    {
        if (mva1.buf[0] != tmp)
        {
            bool accent = (mva1.buf[0] >> 7); // top bit
            bool slide = 1;
            note = mva1.buf[0] & 0x7F;
            
            note_on(note, slide, accent);
        }
    }
    else { note_off(); }
}

void  SynthVoice::note_on(uint8_t midiNote, bool slide, bool accent) 
{
  _accent = accent;
  _slide = slide || _portamento;
  _targetStep = midi_2048_steps[midiNote];
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


/* An old version of Monophonic Voice Allocator by Copych
void SynthVoice::StartNote(uint8_t midiNote, uint8_t velo) {
//  _midiNote = midiNote;
  if (midiNotes[0] != -1) {
    midiNotes[1] = midiNotes[0];
    _slide = true;
  }
  midiNotes[0] = midiNote;
  _targetStep = midi_2048_steps[midiNote];
  _accent = (velo > 80);
  if ( _eAmpEnvState != ENV_IDLE && _eAmpEnvState != ENV_RELEASE ) {
    _slide = true;
  } else {
    _slide = false;
  }
  if (_slide || _portamento) {
    _deltaStep = (_targetStep - _currentStep) * (1000.0f * DIV_SAMPLE_RATE / _slideMs );
  } else {
    _currentStep = _targetStep;
    _deltaStep = 0.0f ;
    _eAmpEnvState = ENV_INIT;
    _eFilterEnvState = ENV_INIT;
  }
}

void SynthVoice::EndNote(uint8_t midiNote, uint8_t velo) {
  if (midiNotes[1] == -1) {
    if (midiNotes[0] == midiNote) {
      _eAmpEnvState = ENV_RELEASE;
      _ampEnvPosition = 0;
      midiNotes[0] = -1;
    }
  } else {
    if (midiNotes[0] == midiNotes[1] ) {
      if (midiNotes[0] == midiNote) {
        _eAmpEnvState = ENV_RELEASE;
        _ampEnvPosition = 0;
        midiNotes[1] = -1;
      }
    } else {
      if (midiNotes[0] == midiNote) {
        _eAmpEnvState = ENV_RELEASE;
        _ampEnvPosition = 0;
        _filterEnvPosition = 0;
        midiNotes[1] = -1;
      }
    }
  }
}
*/
