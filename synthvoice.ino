#include "synthvoice.h"


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



inline void SynthVoice::Generate() {
  float samp = 0.0f, saw_blep = 0.0f, sqr_blep = 0.0f, filtEnv = 0.0f, ampEnv = 0.0f, final_cut = 0.0f;
  for (int i = 0; i < DMA_BUF_LEN; ++i) {
  prescaler++;
  filtEnv = GetFilterEnv();
  ampEnv = GetAmpEnv();
//  filtEnv = filtDeclicker.Process(filtEnv);
//  ampEnv = ampDeclicker.Process(ampEnv);
    if (_eAmpEnvState != ENV_IDLE) {
      //samp = (((1.0f - _waveMix) * (square_2048[ (uint16_t)(_phaze) ] ) ) + ( _waveMix * exp_2048[ (uint16_t)(_phaze) ] )); // lookup and mix waveforms
      samp = (1.0f - _waveMix) * lookupTable(square_2048,_phaze) + _waveMix * lookupTable(exp_2048,_phaze) ; // lookup and mix waveforms
    } else {
      samp = 0.0f;
    }
    
    final_cut = MIN_CUTOFF_FREQ + (MAX_CUTOFF_FREQ - MIN_CUTOFF_FREQ) * (_cutoff * (1.0f - 0.2f * _envMod) + _envMod * (filtEnv - 0.15f));

    if (prescaler % 4) Filter.SetCutoff( final_cut );

    samp = Filter.Process(samp); 
    samp *= ampEnv ;
    samp = Drive.Process(samp);
    samp *=  volume;
//   if ( prescaler % DMA_BUF_LEN*2 == 0 && _index == 0 ) DEBUG( final_cut );
    
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
    synth_buf[_index][i] = fast_tanh(samp); // mono
  }
}

void SynthVoice::Init() {
  Filter.Init((float)SAMPLE_RATE);  
  Drive.Init();
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
      _reso = cc_value * MIDI_NORM * 0.96f;
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
      _gain = 0.125f + (float)cc_value * MIDI_NORM * (0.875f);
      Drive.SetDrive(_gain ); 
      break;
    case CC_303_SATURATOR:
      Filter.SetDrive((float)cc_value * MIDI_NORM * 5.0);
      break;
  }
}

float SynthVoice::GetAmpEnv() {
  const static float sust_level = 0.25f;
  const static float k_sust = 1.0f - sust_level;
  static float ret_val = 0.0f;
  static float pass_val = 0.0f, release_lvl = 0.0f;  
  switch (_eAmpEnvState) {
    case ENV_INIT:
      _ampEnvPosition = 0;
      _ampEnvAttackStep = _msToSteps * one_div( _ampAttackMs+0.0001);
      _ampEnvDecayStep = _msToSteps * one_div(_ampDecayMs+0.0001);
      _ampEnvReleaseStep = _msToSteps * one_div(_ampReleaseMs+0.0001);
      if (_accent) {
        _ampEnvAttackStep *= 1.4f;
        _ampEnvDecayStep *= 1.6f;
      }
      _eAmpEnvState = ENV_ATTACK;
      ret_val = (-exp_2048[ 0 ] + 1.0f) * 0.5f;
      break;
    case ENV_ATTACK:
      _ampEnvPosition += _ampEnvAttackStep;
      if (_ampEnvPosition >= WAVE_SIZE) {
        _eAmpEnvState = ENV_DECAY;
        _ampEnvPosition = 0;
        ret_val = (-exp_2048[ WAVE_SIZE-1 ] + 1.0f) * 0.5f;
      } else {
        ret_val = (-lookupTable(exp_2048, _ampEnvPosition ) + 1.0f) * 0.5f;
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
        ret_val = sust_level + k_sust * (lookupTable(exp_2048, _ampEnvPosition) + 1.0f) * 0.5f;
      }
      pass_val = ret_val;
      break;
    case ENV_SUSTAIN: 
      ret_val = sust_level; // for 303 asuming sustain to be endless
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
        ret_val = release_lvl * (lookupTable(exp_2048, _ampEnvPosition) + 1.0f) * 0.5f;
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
  return ret_val;
}

inline float SynthVoice::GetFilterEnv() {
  static float ret_val = 0.0f;
  switch(_eFilterEnvState) {
    case ENV_INIT:
      _offset = ret_val;
      _filterEnvPosition = 0.0f;
      _filterEnvAttackStep = _msToSteps * one_div(_filterAttackMs+0.0001);
      _filterEnvDecayStep = _msToSteps * one_div(_filterDecayMs+0.0001);
      if (_accent) {
        _filterEnvAttackStep *= 1.4f;
        _filterEnvDecayStep *= 1.8f;
      }
      _eFilterEnvState = ENV_ATTACK;
      ret_val = (-exp_2048[ 0 ] + 1.0f) * 0.5f;
      break;
    case ENV_ATTACK:
      if (_filterEnvPosition >= (float)WAVE_SIZE) {
        _eFilterEnvState = ENV_DECAY;
        _filterEnvPosition = 0.0f;
        ret_val = (-exp_2048[ WAVE_SIZE-1 ] + 1.0f) * 0.5f;
      } else {
        ret_val = (-lookupTable(exp_2048, _filterEnvPosition) + 1.0f) * 0.5f ;
      }
      _filterEnvPosition += _filterEnvAttackStep;
      ret_val += _offset;
      break;
    case ENV_DECAY:
      if (_filterEnvPosition >= (float)WAVE_SIZE) {
        _eFilterEnvState = ENV_IDLE; // Attack-Decay-0 envelope (?)
        _filterEnvPosition = 0.0f;
        ret_val = 0.0f;
      } else {
        ret_val =  (lookupTable(exp_2048, _filterEnvPosition) + 1.0f) * 0.5f ;
      }
      _filterEnvPosition += _filterEnvDecayStep;
      ret_val *= (1.0f + _offset);
      break;
    case ENV_IDLE:
      ret_val =  0.0f;
      break;
    default:      
      ret_val =  0.0f;
  }
  return ret_val;
}
