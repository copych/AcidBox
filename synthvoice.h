#ifndef TB303VOICE_H
#define TB303VOICE_H

#include "moogladder.h"
//#include "rosic_TeeBeeFilter.h"
//#include "sp_tbvcf.h"
#include "wavefolder.h"
#include "midi_controls.h"

class SynthVoice {
public:
  SynthVoice();
  SynthVoice(uint8_t ind) {_index = ind;};
  void Init();
	void StartNote(uint8_t midiNote, uint8_t velo);
  void EndNote(uint8_t midiNote, uint8_t velo);
  inline void StopSound();
  inline void SetSlideOn()                {_slide=true;};
  inline void SetSlideOff()               {_slide=false;};
  inline void SetAccentOn()               {_accent=true;};
  inline void SetAccentOff()              {_accent=false;};
  inline void SetVolume(uint8_t vol)      {volume = (float)vol;};
  inline void SetPan(uint8_t pan)         {pan = (float)pan;};
  inline void SetDelayLevel(uint8_t lvl)  {_sendDelay = (float)lvl;};
  inline void SetDistortionLevel(uint8_t lvl) {_gain = (float)lvl;};
  inline void SetCutoff(uint8_t lvl) {_cutoff = (float)lvl;};
  inline void SetReso(uint8_t lvl)   {_reso = (float)lvl;};
  inline void SetEnvModLevel(uint8_t lvl) {_envMod = (float)lvl;};
  inline void SetAccentLevel(uint8_t lvl) {_accentLevel = (float)lvl;};
  inline void SetTempo(uint8_t tempo)     {_tempo = (float)tempo;};
  inline void SetIndex(uint8_t ind)       {_index = ind;};
  inline void ParseCC(uint8_t cc_number, uint8_t cc_value);
  inline float GetAmpEnv();                // call once per sample
  inline float GetFilterEnv();             // call once per sample
  inline void Generate() ;
  MoogLadder Filter;
 // rosic::TeeBeeFilter Filter;
  //SP_TBVCF Filter;
  Wavefolder WFolder;
  float pan = 0.5f;
  float volume = 0.5f;
  int midiNotes[2] = {-1, -1};
    
private:
  // most CC controlled values internally are float, nevertheless their range corresponds to MIDI 0-127 (internally 0.0f-1.0f)
  uint8_t _index = 0;
  bool _accent = false;
  bool _slide = false;
  bool _portamento = false; // slide, but managed by CC 65 
  float _tempo = 100.0f;
  float _waveMix = 1.0f ; // square = 0.0f and saw = 1.0f
  float _sampleRate = (float)SAMPLE_RATE;
  uint16_t bufSize = DMA_BUF_LEN;
  float _detuneCents = 0.0f;
  float _envMod = 0.0f;   
  float _accentLevel = 0.0f; 
  float _cutoff = 0.2f; // * 5000 = Hz
  float _reso = 0.4f;
  float _sendDelay = 0.0f;
  float _gain = 1.0; // values >1 will distort sound
  enum eEnvState_t {ENV_IDLE, ENV_INIT, ENV_ATTACK, ENV_DECAY, ENV_SUSTAIN, ENV_RELEASE, ENV_WAITING};
  volatile eEnvState_t _eAmpEnvState = ENV_IDLE;
  volatile eEnvState_t _eFilterEnvState = ENV_IDLE;
  
  uint32_t _noteStartTime = 0;
  uint8_t _midiNote = 69;
  float  _currentStep = 1.0f;
  float  _targetStep = 0.0f;
  float  _deltaStep = 0.0f;
  float  _slideMs = 60.0f;
  float  _phaze = 0.0f;
  
  // MEG and FEG params
  float _ampEnvPosition = 0.0;
  float _filterEnvPosition = 0.0;
  float _ampAttackMs = 3.0;
  float _ampDecayMs = 300.0;
  float _filterAttackMs = 3.0;
  float _filterDecayMs = 200.0;
  float _ampAccentAttackMs = 3.0;
  float _ampAccentDecayMs = 300.0;
  float _filterAccentAttackMs = 3.0;
  float _filterAccentDecayMs = 200.0;
  float _ampEnvAttackStep = 15.0;
  float _ampEnvDecayStep = 1.0;
  float _filterEnvAttackStep = 15.0;
  float _filterEnvDecayStep = 1.0;
  
  float _msToSteps = (float)WAVE_SIZE * DIV_SAMPLE_RATE * 1000.0f;
};

#endif
