#pragma once
#ifndef SYNTHVOICE_H
#define SYNTHVOICE_H

#include "tables.h"
#include "../filter/adsr.h"
#include "../filter/ad.h"
#include "../fx/overdrive.h"

#define FILTER_TYPE 2       // 0 = Moogladder by Victor Lazzarini
                            // 1 = Tim Stilson's model by Aaron Krajeski
							              // 2 = Open303 filter
/* 
 *  You shouldn't need to change something below this line
*/


#define MIDI_MVA_SZ 8
#if FILTER_TYPE == 0
#include "../filter/moogladder.h"
#endif
#if FILTER_TYPE == 1
#include "../filter/krajeski_flt.h"
#endif
#if FILTER_TYPE == 2
#include "../filter/rosic_TeeBeeFilter.h"
#endif

#include "../filter/rosic_OnePoleFilter.h"
#include "../filter/rosic_BiquadFilter.h"

#include "wavefolder.h"

//#include "fx_rat.h"

#include "midi_config.h"
//#include "smoother.h"

typedef struct 
{
  uint8_t notes[MIDI_MVA_SZ];
  bool accents[MIDI_MVA_SZ];
  uint8_t n;
} mva_data;

class SynthVoice {
public:
  static float constexpr MIN_CUTOFF_FREQ = 300.0;
  static float constexpr MAX_CUTOFF_FREQ = 3530.0;
  static float constexpr MIN_CUTOFF_FREQ_MOD = 2740.0;
  static float constexpr MAX_CUTOFF_FREQ_MOD = 19300.0;
  size_t      decimator = 0; // debugging only needs!
  Adsr        AmpEnv;
  AD_env      FltEnv;
  SynthVoice(uint8_t index, volatile int *_current_gen_buf);
  void Init();
  void generate() __attribute__((noinline)); 
  float WORD_ALIGNED_ATTR  synth_buf[2][DMA_BUF_LEN]  = {0.0f};

  // inline //
  inline void on_midi_noteON(uint8_t note, uint8_t velocity);
  inline void on_midi_noteOFF(uint8_t note, uint8_t velocity);
  inline void SetSlideOn();
  inline void SetSlideOff();
  inline void SetVolume(float val);
  inline void SetPan(float pan);
  inline void SetDelaySend(float lvl);
  inline void SetReverbSend(float lvl);
  inline void SetDistortionLevel(float lvl);
  inline void SetOverdriveLevel(float lvl);
  inline void SetCutoff(float lvl);
  inline void SetReso(float lvl);
  inline void SetEnvModLevel(float lvl);
  inline void SetAccentLevel(float lvl);
  inline void SetTempo(float tempo);
  inline void SetIndex(uint8_t ind);
  inline void ParseCC(uint8_t cc_number, uint8_t cc_value);
  inline void PitchBend(int number) ;
  inline void allNotesOff();
  inline float GetPan();
  inline float GetVolume();
  inline float getSample();

// inline //


  float _sendDelay = 0.0f;
  float _sendReverb = 0.0f;
  int WORD_ALIGNED_ATTR  midiNotes[2] = {-1, -1};

  mva_data mva1;

#if FILTER_TYPE == 0
  MoogLadder        Filter;
#endif
#if FILTER_TYPE == 1
  KrajeskiMoog      Filter;
#endif
#if FILTER_TYPE == 2
  TeeBeeFilter      Filter;
#endif

  
private:
  // Pointer to buffer
  volatile int *_current_gen_buf;

  // most CC controlled values internally are float, nevertheless their range maps to MIDI 0-127 (internally 0.0f-1.0f)
  float WORD_ALIGNED_ATTR  _tempo = 100.0f;
  float _waveMix = 1.0f ; // exp-square = 0.0f and exp-saw = 1.0f
  int _waveBase = 0; // calculate pointers to the two tables to blend
  float _sampleRate = (float)SAMPLE_RATE;
  int bufSize = DMA_BUF_LEN;
  float _detuneCents = 0.0f;
  float _envMod = 0.5f;
  float _accentLevel = 0.5f;
  float _accentation = 0.0f;
  float _cutoff = 0.2f;   // 0..1 normalized freq range. Keep in mind that EnvMod set to max practically doubles this range
  float _filter_freq = 500.0f; // cutoff freq, Hz
  float _filter_freq_mod = 2740.0f; // cutoff freq, Hz with envMod up
  float _filter_freq_cut = 500.0f;
  float _filt_avg = 400.0f;
  float _reso = 0.4f; // normalized
  float _saturator = 0.0; // pre shaper
  float _gain = 0.0;      // post distortion, cc94
  float _drive = 0.0;      // post overdrive, cc95
  
  float _pan = 0.5f;
  float _volume = 1.0f;
  
  uint32_t _noteStartTime = 0;
  float  _currentStep = 1.0f;
  float  _subStep = 4.0f;
  int    _wave_cnt = 0;
  float  _currentPeriod = 1.0f; // should be int (trying to come exactly to 0 phase)
  float  _avgStep = 1.0f;
  float  _avgPeriod = 1.0f; // measured in samples
  float  _targetStep = 1.0f;
  float  _targetPeriod = 0.0f;
  float  _deltaStep = 0.0f;
  float  _slideMs = 60.0f;
  float  _phaze = 0.0f;
  float  _tuning = 1.0f;
  float  _effectiveStep = 1.0f;
  float  _pitchbend = 1.0f;
  
  // parameters of envelopes
  float _sust_level = 0.2f;
  float _release_lvl = 0.0f;
  float _pass_val = 0.0f;
  float _k_acc = 1.0f;
  float _ampEnvVal = 0.0f;
  float _filterEnvVal = 0.0f;
  float _ampEnvPosition = 0.0f;
  float _filterEnvPosition = 0.0f;
  float _ampAttackMs = 3.0f;
  float _ampDecayMs = 300.0f;
  float _ampReleaseMs = 30.0f;
  float _ampEnvAttackStep = 15.0f;
  float _ampAccentReleaseMs = 50.0f;
  float _filterAttackMs = 3.0f;
  float _filterDecayMs = 1000.0f;
  float _filterAccentDecayMs = 200.0f;
  float _filterAccentAttackMs = 50.0f;
  float _offset = 0.0f; // filter discharge 
  float _offset_leak = 0.9999f; 
  float _msToSteps = (float)TABLE_SIZE * DIV_SAMPLE_RATE * 1000.0f;
  float _compens = 1.0f;
  float _fx_compens = 1.0f;
  float _flt_compens = 1.0f;
  void mva_note_on(mva_data *p, uint8_t note, uint8_t accent);
  void mva_note_off(mva_data *p, uint8_t note);
  void mva_reset(mva_data *p);
  void note_off() ;
  void note_on(uint8_t midiNote, bool slide, bool accent) ;
  void calcEnvModScalerAndOffset();
 // Smoother          ampDeclicker;
 // Smoother          filtDeclicker;
  uint8_t _midiNote = 69;
  uint8_t _index = 0;
  bool _accent = false;
  bool _slide = false;
  bool _portamento = false; // slide, but managed by CC 65 

  BiquadFilter      ampDeclicker;
  BiquadFilter      filtDeclicker;
  
  BiquadFilter      notch;        //taken from open303, subj to check
  OnePoleFilter     highpass1;  
  OnePoleFilter     highpass2;
  OnePoleFilter     allpass; 
  
  Wavefolder        Distortion;
//  RAT               Distortion;
  
  Overdrive         Drive;
};

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

inline void SynthVoice::SetSlideOn() {
  _slide=true;
};

inline void SynthVoice::SetSlideOff() {
  _slide=false;
};

inline void SynthVoice::SetVolume(float val) {
  _volume = val;
};

inline void SynthVoice::SetPan(float pan) {
  _pan = pan;
};

inline void SynthVoice::SetDelaySend(float lvl) {
  _sendDelay = lvl;
};

inline void SynthVoice::SetReverbSend(float lvl)  {
  _sendReverb = lvl;
};

inline void SynthVoice::SetDistortionLevel(float lvl) {
  _gain = lvl;
  Distortion.SetDrive(_gain );
};

inline void SynthVoice::SetOverdriveLevel(float lvl) {
  _drive = lvl;  
  Drive.SetDrive(_drive ); 
};

inline void SynthVoice::SetCutoff(float normalized_val)  {
  _cutoff = normalized_val;
  _filter_freq = General::knobMap( normalized_val, MIN_CUTOFF_FREQ, MAX_CUTOFF_FREQ);
  _filter_freq_mod = General::knobMap( normalized_val, MIN_CUTOFF_FREQ_MOD, MAX_CUTOFF_FREQ_MOD);
  _filter_freq_cut = General::knobMap( _envMod, _filter_freq, _filter_freq_mod);
#ifdef DEBUG_SYNTH
  DEBF("Synth %d cutoff=%0.3f freq=%0.3f\r\n" , _index, _cutoff, _filter_freq);
#endif
}

inline void SynthVoice::SetReso(float lvl) {
    _reso = constrain(lvl, 0.0f, 1.0f);
    Filter.SetResonance(_reso); 
};

inline void SynthVoice::SetEnvModLevel(float normalized_val) {
  _envMod = normalized_val;
  _filter_freq_cut = General::knobMap( normalized_val, _filter_freq, _filter_freq_mod);
};

inline void SynthVoice::SetAccentLevel(float lvl) {
  _accentLevel = lvl;
};

inline void SynthVoice::SetTempo(float tempo) {
  _tempo = tempo;
};

inline void SynthVoice::SetIndex(uint8_t ind) {
  _index = ind;
};

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
      _flt_compens = General::one_div( Tables::bilinearLookup(Tables::norm1_tbl, _cutoff * 127.0f, cc_value ));
      SetReso(_reso);
      break;
    case CC_303_DECAY: // Env release
      tmp = (float)cc_value * MIDI_NORM;
      _filterDecayMs = General::knobMap(tmp, 200.0f, 2000.0f);
      //_ampDecayMs = General::knobMap(tmp, 15.0f, 7500.0f);
      break;
    case CC_303_ATTACK: // Env attack
      tmp = (float)cc_value * MIDI_NORM;
      _filterAttackMs = General::knobMap(tmp, 3.0f, 100.0f);
      _ampAttackMs =  General::knobMap(tmp, 0.1f, 500.0f);
      break;
    case CC_303_CUTOFF:
      _cutoff = (float)cc_value * MIDI_NORM;
      _flt_compens = General::one_div( Tables::bilinearLookup(Tables::norm1_tbl, cc_value, _reso * 127.0f));
      SetCutoff(_cutoff);
      break;
    case CC_303_DELAY_SEND:
      _sendDelay = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_REVERB_SEND:
      _sendReverb = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_ENVMOD_LVL:
      SetEnvModLevel ( (float)cc_value * MIDI_NORM ) ;
      break;
    case CC_303_ACCENT_LVL:
      _accentLevel = (float)cc_value * MIDI_NORM;
      break;
    case CC_303_DISTORTION:
      _gain = (float)cc_value * MIDI_NORM ;
      _fx_compens = General::one_div( Tables::bilinearLookup(Tables::norm2_tbl, _drive * 127.0f,  cc_value));
      SetDistortionLevel(_gain);
      break;
    case CC_303_OVERDRIVE:
      _drive = (float)cc_value * MIDI_NORM ;
      _fx_compens = General::one_div( Tables::bilinearLookup(Tables::norm2_tbl, cc_value, _gain * 127.0f));
      SetOverdriveLevel(_drive);
      break;
    case CC_303_SATURATOR:
      _saturator = (float)cc_value * MIDI_NORM;
      Filter.SetDrive(_saturator);
      break;
    case CC_303_TUNING:
      _tuning = tuning[cc_value];
      _effectiveStep = _targetStep * _tuning * _pitchbend;
      break;
  }
}

inline void SynthVoice::PitchBend(int number) {
  //
  float semi = ((((float)number + 8191.5f) * (float)TWO_DIV_16383 ) - 1.0f ) * 12.0f;
  _pitchbend = powf(1.059463f, semi);
  _effectiveStep = _targetStep * _tuning * _pitchbend;
}

inline void SynthVoice::allNotesOff() {
  mva1.n=0;
  AmpEnv.end(Adsr::END_FAST);
  FltEnv.end(false);
};

inline float SynthVoice::GetPan() {
  return _pan;
}

inline float SynthVoice::GetVolume() {
  return _volume;
}

inline float SynthVoice::getSample() {
  
    float samp = 0.0f, filtEnv = 0.0f, ampEnv = 0.0f, final_cut = 0.0f;
    filtEnv = FltEnv.process();
    
    ampEnv = AmpEnv.process() * _k_acc;
    
    if (AmpEnv.isRunning()) {
      // samp = (float)((1.0f - _waveMix) *Tables::lookupTable (*(tables[_waveBase]), _phaze)) + (float)(_waveMix * Tables::lookupTable(*(tables[_waveBase+1]), _phaze)) ; // lookup and blend waveforms
       samp = (float)((1.0f - _waveMix) * Tables::lookupTable(Tables::exp_square_tbl, _phaze)) + (float)(_waveMix * Tables::lookupTable(Tables::saw_tbl, _phaze)) ; // lookup and blend waveforms
      // samp = _phaze < HALF_TABLE ? 2 * _waveMix * _phaze * DIV_TABLE_SIZE - 1.0f   :    2 * _waveMix * (_phaze * DIV_TABLE_SIZE - 1.0f) + 1.0f; 
    } else {
      samp = 0.0f;
    }
    final_cut = (float)_filter_freq_cut * (0.8f + (_envMod+0.1f) * (3*filtEnv - 0.3f) * (_accentation + 0.2f) );
    //final_cut = (float)_filter_freq * ( (float)_envMod * ((float)filtEnv - 0.2f) + 1.3f * (float)_accentation + 1.0f );
//    final_cut = filtDeclicker.getSample( final_cut );
    Filter.SetCutoff( final_cut );    

    
     decimator++;
     if (decimator % 128 == 0 && _index == 0) {
      // TODO, check what this is, this sends load of data when turning debug on
      //DEBF("%f\r\n", final_cut);
     }
    
    samp = highpass1.getSample(samp);         // pre-filter highpass, following open303
    
    samp = allpass.getSample(samp);           // phase correction, following open303
   
    samp = Filter.Process(samp);              // main filter
    
    samp = highpass2.getSample(samp);         // post-filtering, following open303
    
    samp = notch.getSample(samp);             // post-filtering, following open303
    
    samp = Drive.Process(samp);               // overdrive
    
    samp = Distortion.Process(samp);          // distortion
    
    samp *= ampEnv;                           // amp envelope


    _compens = _volume * 8.0f * _fx_compens ; // * _flt_compens;

    _compens = ampDeclicker.getSample(_compens);
   
    samp *=  _compens;

    if ((_slide || _portamento) && _deltaStep != 0.0f) {     // portamento / slide processing
      if (fabs(_effectiveStep - _currentStep) >= fabs(_deltaStep)) {
        _currentStep += _deltaStep;
      } else {
        _currentStep = _effectiveStep;
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
    
    //synth_buf[_index][i] = General::fast_shape(samp); // mono limitter
    return  samp;  
}

#endif