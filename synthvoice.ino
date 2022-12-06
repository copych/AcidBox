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
  if ( _eAmpEnvState != ENV_IDLE ) {
    _slide = true;
  } else {
    _slide = false;
  }
  if (_slide || _portamento) {
    _deltaStep = (_targetStep - _currentStep) * (1000.0f * DIV_SAMPLE_RATE / _slideMs );
  //  _eAmpEnvState = ENV_INIT;
  //  _eFilterEnvState = ENV_INIT;
  } else {
    _currentStep = _targetStep;
    _deltaStep = 0.0f ;
    _eAmpEnvState = ENV_INIT;
    _eFilterEnvState = ENV_INIT;
  }
 // Filter.SetFreq(_cutoff * 5000.0f);
 // Filter.SetRes(_reso);
}

void SynthVoice::EndNote(uint8_t midiNote, uint8_t velo) {
  if (midiNotes[1] == -1) {
    if (midiNotes[0] == midiNote) {
      _eAmpEnvState = ENV_IDLE;
      midiNotes[0] = -1;
    }
  } else {
    if (midiNotes[0] == midiNotes[1] ) {
      if (midiNotes[0] == midiNote) {
        _eAmpEnvState = ENV_IDLE;
        midiNotes[1] = -1;
      }
    } else {
      if (midiNotes[0] == midiNote) {
        _eAmpEnvState = ENV_IDLE;
        midiNotes[1] = -1;
      }
    }
  }
}

void SynthVoice::Generate() {
  float samp = 0.0f;
  float amplitude = 0.0f;
  for (int i = 0; i < DMA_BUF_LEN; ++i) {
    if (_eAmpEnvState != ENV_IDLE) {
      samp =  GetAmpEnv() * (((1.0f - _waveMix) * square_2048[ (uint16_t)(_phaze) ] ) + ( _waveMix * saw_2048[ (uint16_t)(_phaze) ] ));
    } else {
      samp = 0.0f;
    }
    // Filter
//    Filter.SetFreq(5000.0f * (_cutoff +  GetFilterEnv()));
    //Filter.setFeedbackHighpassCutoff(_cutoff * 5000.0f);
    Filter.setCutoff(5000.0f * (_cutoff +  _envMod*GetFilterEnv()));
    //samp = Filter.Process(samp); // moogladder
    //samp = Filter.getSample(samp); // rosic open303
    samp = Filter.Process(samp); // TBVCF
    samp = WFolder.Process(samp);
    if ((_slide || _portamento) && _deltaStep!=0.0f) {
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
    synth_buf[_index][i] = (samp);
    //synth_buf[_index][i+DMA_BUF_LEN] = (samp);
  }
}

void SynthVoice::Init() {
/*
  Filter.Init(SAMPLE_RATE);
  Filter.SetFreq(_cutoff * 5000.0f);
  Filter.SetRes(_reso);
*/
//  Filter.setSampleRate(SAMPLE_RATE);
//Filter.setMode(rosic::TeeBeeFilter::TB_303);
  //Filter.setResonance(_reso);
 // Filter.setFeedbackHighpassCutoff(150.0f);
 // Filter.setCutoff(_cutoff * 5000.0f);
  Filter.Init((float)SAMPLE_RATE);
  WFolder.Init(); 
}


inline void SynthVoice::ParseCC(uint8_t cc_number , uint8_t cc_value) {
  switch (cc_number) {
    
    case CC_303_PORTATIME:
      _slideMs = cc_value;
      break;
    case CC_303_VOLUME:
      volume = cc_value * MIDI_NORM;
      break;
    case CC_303_PAN:
      pan = cc_value * MIDI_NORM;
      break; 
    case CC_303_PORTAMENTO:
      _portamento = (cc_value >= 64); 
      break; 
    case CC_303_WAVEFORM:
      _waveMix = cc_value * MIDI_NORM;
      break;
    case CC_303_RESO:
      _reso = cc_value * MIDI_NORM * 0.96f;
      Filter.setResonance(_reso);
      break;    
    case CC_303_DECAY: // Env release
      _filterDecayMs = 5.0f + cc_value * MIDI_NORM * 5000.0f ;
      _ampDecayMs = 5.0f + cc_value * MIDI_NORM * 7500.0f;
      _filterAccentDecayMs = 0.5 * _ampDecayMs;
      _ampAccentDecayMs = 0.5 * _ampDecayMs;
      break;
    case CC_303_ATTACK: // Env attack
      _filterAttackMs = cc_value * MIDI_NORM * 500.0f ;
      _ampAttackMs =  cc_value * MIDI_NORM * 700.0f;
      _filterAccentAttackMs = 0.3f * _filterAttackMs ;
      _ampAccentAttackMs = 0.3f * _ampAttackMs;
      break;
    case CC_303_CUTOFF:
      _cutoff = cc_value * MIDI_NORM;
      break;
    case CC_303_DELAYSEND:
      _sendDelay = cc_value * MIDI_NORM;
      break;
    case CC_303_ENVMOD_LVL:
      _envMod = cc_value * MIDI_NORM;
      break;
    case CC_303_ACCENT_LVL:
      _accentLevel = cc_value * MIDI_NORM;
      break;
    case CC_303_DISTORTION:
      _gain = cc_value * 60.0f * MIDI_NORM;
      WFolder.SetGain(_gain + 1.0f); 
      break;
  }
}

inline float SynthVoice::GetAmpEnv() {
  if (_eAmpEnvState == ENV_INIT) {
    _ampEnvPosition = 0;
    if (!_accent) {
      _ampEnvAttackStep = _msToSteps / _ampAttackMs;
      _ampEnvDecayStep = _msToSteps / _ampDecayMs;
    } else {
      _ampEnvAttackStep = _msToSteps / _ampAccentAttackMs;
      _ampEnvDecayStep = _msToSteps / _ampAccentDecayMs;
    }
    _eAmpEnvState = ENV_ATTACK;
    return (-saw_2048[ 0 ] + 1.0f) * volume;
  }
  if (_eAmpEnvState == ENV_ATTACK) {
    _ampEnvPosition += _ampEnvAttackStep;
    if (_ampEnvPosition >= WAVE_SIZE) {
      _eAmpEnvState = ENV_DECAY;
      _ampEnvPosition = 0;
      return (-saw_2048[ WAVE_SIZE-1 ] + 1.0f) * volume;
    } else {
      return (-saw_2048[ (uint16_t)_ampEnvPosition ] + 1.0f) * volume ;
    }
  }
  if (_eAmpEnvState == ENV_DECAY) {
    _ampEnvPosition += _ampEnvDecayStep;
    if (_ampEnvPosition >= WAVE_SIZE) {
      _eAmpEnvState = ENV_IDLE;
      return 0.0f;
    } else {
      return (saw_2048[ (uint16_t)_ampEnvPosition ] + 1.0f) * volume ;
    }
  }
}


inline float SynthVoice::GetFilterEnv() {
  if (_eFilterEnvState == ENV_INIT) {
    _filterEnvPosition = 0;
    if (!_accent) {
      _filterEnvAttackStep = _msToSteps / _filterAttackMs;
      _filterEnvDecayStep = _msToSteps / _filterDecayMs;
    } else {
      _filterEnvAttackStep = _msToSteps / _filterAccentAttackMs;
      _filterEnvDecayStep = _msToSteps / _filterAccentDecayMs;
    }
    _eFilterEnvState = ENV_ATTACK;
    return (-saw_2048[ 0 ] + 1.0f) * 0.5f;
  }
  if (_eFilterEnvState == ENV_ATTACK) {
    _filterEnvPosition += _filterEnvAttackStep;
    if (_filterEnvPosition >= WAVE_SIZE) {
      _eFilterEnvState = ENV_DECAY;
      _filterEnvPosition = 0.0f;
      return (-saw_2048[ WAVE_SIZE-1 ] + 1.0f) * 0.5f;
    } else {
      return (-saw_2048[ (uint16_t)_filterEnvPosition ] + 1.0f) * 0.5f ;
    }
  }
  if (_eFilterEnvState == ENV_DECAY) {
    _filterEnvPosition += _filterEnvDecayStep;
    if (_filterEnvPosition >= WAVE_SIZE) {
      _eFilterEnvState = ENV_IDLE;
      return 0.0f;
    } else {
      return (saw_2048[ (uint16_t)_filterEnvPosition ] + 1.0f) * 0.5f ;
    }
  }
}
