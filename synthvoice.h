#pragma once
#include "adsr.h"
#include "ad.h"

#define FILTER_TYPE 2       // 0 = Moogladder by Victor Lazzarini
                            // 1 = Tim Stilson's model by Aaron Krajeski
							              // 2 = Open303 filter
/* 
 *  You shouldn't need to change something below this line
*/


#define MIDI_MVA_SZ 8
#if FILTER_TYPE == 0
#include "moogladder.h"
#endif
#if FILTER_TYPE == 1
#include "krajeski_flt.h"
#endif
#if FILTER_TYPE == 2
#include "rosic_TeeBeeFilter.h"
#endif

#include "rosic_OnePoleFilter.h"
#include "rosic_BiquadFilter.h"

#include "wavefolder.h"
#include "overdrive.h"
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
  static float constexpr MIN_CUTOFF_FREQ = 500.0;
  static float constexpr MAX_CUTOFF_FREQ = 3530.0;
  static float constexpr MIN_CUTOFF_FREQ_MOD = 2740.0;
  static float constexpr MAX_CUTOFF_FREQ_MOD = 19300.0;
  size_t      decimator = 0; // debugging only needs!
  Adsr        AmpEnv;
  AD_env      FltEnv;
  SynthVoice();
  SynthVoice(uint8_t ind) {_index = ind;};
  void Init();
  inline void on_midi_noteON(uint8_t note, uint8_t velocity);
  inline void on_midi_noteOFF(uint8_t note, uint8_t velocity);
  inline void StopSound();
  inline void SetSlideOn()                {_slide=true;};
  inline void SetSlideOff()               {_slide=false;};
  inline void SetVolume(float val)      {_volume = val;};
  inline void SetPan(float pan)         {_pan = pan;};
  inline void SetDelaySend(float lvl)   {_sendDelay = lvl;};
  inline void SetReverbSend(float lvl)  {_sendReverb = lvl;};
  inline void SetDistortionLevel(float lvl) {_gain = lvl; Distortion.SetDrive(_gain ); };
  inline void SetOverdriveLevel(float lvl) {_drive = lvl;  Drive.SetDrive(_drive ); };
  inline void SetCutoff(float lvl);
  inline void SetReso(float lvl)        {_reso = constrain(lvl, 0.0f, 1.0f); Filter.SetResonance(_reso); };
  inline void SetEnvModLevel(float lvl);
  inline void SetAccentLevel(float lvl) {_accentLevel = lvl;};
  inline void SetTempo(float tempo)     {_tempo = tempo;};
  inline void SetIndex(uint8_t ind)       {_index = ind;};
  inline void ParseCC(uint8_t cc_number, uint8_t cc_value);
  inline void PitchBend(int number) ;
  inline void allNotesOff()               {mva1.n=0;  AmpEnv.end(Adsr::END_FAST); FltEnv.end(false);};
  inline float GetAmpEnv();                // call once per sample
  inline float GetFilterEnv();             // call once per sample
  inline float GetPan()                 {return _pan;}
  inline float GetVolume()              {return _volume;}
  inline float getSample() ;
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
  inline void calcEnvModScalerAndOffset();
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
