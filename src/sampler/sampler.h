#ifndef SAMPLER_H
#define SAMPLER_H

#include <FS.h>
#include <LittleFS.h>
#include "midi_config.h"
#include "../fx/fx_filtercrusher.h"
#include "../general/general.h"

class Sampler {
  public:
    Sampler(uint8_t progNow, volatile int *current_gen_buf);
    void Init();
    void generate() __attribute__((noinline)); 
    float WORD_ALIGNED_ATTR  drums_buf_l[2][DMA_BUF_LEN]  = {0.0f};
    float WORD_ALIGNED_ATTR  drums_buf_r[2][DMA_BUF_LEN]  = {0.0f};
    void ScanContents(fs::FS &fs, const char *dirname, uint8_t levels);

    // Inline
    inline void SelectNote(uint8_t note) __attribute__((always_inline));
    inline void SetNotePan_Midi( uint8_t data1) __attribute__((always_inline));
    inline void SetNoteOffset_Midi(uint8_t data1) __attribute__((always_inline));
    inline void SetNoteDecay_Midi(uint8_t data1) __attribute__((always_inline)); 
    inline void SetNoteVolume_Midi(uint8_t data1) __attribute__((always_inline));
    inline void SetSoundPitch_Midi(uint8_t data1) __attribute__((always_inline));
    inline void SetSoundPitch(float value) __attribute__((always_inline));   
    inline void SetDelaySend(uint8_t lvl) __attribute__((always_inline));
    inline void SetReverbSend(uint8_t lvl) __attribute__((always_inline));
    // Inline
    
    
    uint16_t GetSoundSamplerate() { return samplePlayer[ selectedNote ].sampleRate; };
    uint8_t GetSoundDecay_Midi()  { return samplePlayer[ selectedNote ].decay_midi; };
    uint16_t GetSoundPan_Midi()   { return samplePlayer[ selectedNote ].pan_midi; };
    uint8_t GetSoundPitch_Midi()  { return samplePlayer[ selectedNote ].pitch_midi; };
    uint8_t GetSoundVolume_Midi() { return samplePlayer[ selectedNote ].volume_midi; };
    int32_t GetSamplesCount()     { return sampleInfoCount; }
    // Offset   for the Sample-Playback to cut the sample from the left
    
    // Inline
    inline void NoteOn(uint8_t note, uint8_t vol) __attribute__((always_inline));
    inline void NoteOff(uint8_t note) __attribute__((always_inline));
    // Inline
    
    
    void SetPlaybackSpeed_Midi( uint8_t value ){  SetSoundPitch( (float) MIDI_NORM * value ); };
    void SetPlaybackSpeed( float value );
    void SetProgram( uint8_t prog );
    void SetVolume( float value ) { _volume = value; };
    inline void Process(float *left, float *right) __attribute__((always_inline));
    inline void ParseCC(uint8_t cc_number, uint8_t cc_value) __attribute__((always_inline));
    inline void PitchBend(int number) __attribute__((always_inline));
    float _sendReverb = 0.0f;
    float _sendDelay = 0.0f;
  
  private:
    // Pointer to buffer
    volatile int *_current_gen_buf;

    void CreateDefaultSamples(fs::FS &fs);
    void WriteFile(fs::FS &fs, const String fname, size_t fsize, const uint8_t bytearray[] );
    boolean is_muted[17]={ false, false,false,false,false ,false,false,false,false ,false,false,false,false ,false,false,false,false };
                  
    uint8_t volume_midi[17]     = { 127, 127,127,127,127, 127,127,127,127, 127,127,127,127, 127,127,127,127 };
    uint8_t offset_midi[17]     = { 0, 0,0,0,0,0,0,0,0, 0,0,0,0,0,0,0,0 };
    uint8_t decay_midi[17]      = { 100, 100,100,100,100, 100,100,100,100, 100,100,100,100, 100,100,100,100  };
    uint8_t pitch_midi[17]      = { 64, 64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64 };
    uint8_t pan_midi[17]        = { 64, 64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64 };
    uint8_t pitchdecay_midi[17] = { 64, 64,64,64,64, 64,64,64,64, 64,64,64,64, 64,64,64,64 };
    String shortInstr[17] ={ "ACC", "111","222","333","HHop", "Cr","Cl","LT","HT", "S1","S2","S3","S4", "T1","T2","T3","T4" };
    // Soundset/Program-Settings
    uint8_t  program_midi = 0; 
    uint8_t  program_tmp = DEFAULT_DRUMKIT; 
    uint8_t  progNumber = DEFAULT_DRUMKIT; 
    uint8_t  repeat = 12; // repeat instruments every ....
    float _volume = 1.0f;
    float sampler_playback = 1.0f;
    volatile uint8_t selectedNote = 0;
    // union is very handy for easy conversion of bytes to the wav header information
    union wavHeader{
        struct{
            char riff[4];						// 4
            uint32_t fileSize; 					// 8
            char waveType[4];					// 12
            char format[4];						// 16
            uint32_t lengthOfData;				// 20
            uint16_t numberOfChannels;			// 22
            uint16_t audioFormat;				// 24
            uint32_t sampleRate;				// 28
            uint32_t byteRate;					// 32
            uint16_t bytesPerSample;			// 34
            uint16_t bitsPerSample;				// 36
            char dataStr[4];					// 40
            uint32_t dataSize; 					// 44
        };
        uint8_t wavHdr[44];
    };

    typedef struct samplePlayerS{
   //     char filename[32]; // move it out of the struct in hope to speed up the sampler
   //     File file;
        uint32_t sampleRate; 
        uint32_t sampleStart; // in a PSRAM common buffer;
        uint32_t sampleSize;
        float samplePosF;
        uint32_t samplePos;
     //   uint32_t lastDataOut; 
        bool active;
        uint32_t sampleSeek;
    //    uint32_t dataIn;
        float volume; // Volume of Track
        float signal;
        float decay;
        float vel;  // temp Velocity of NoteOn?
        float pitch;
        // float release;
        float pan;
        // float volume_float; // Volume of Track 0.0 - ?
        
        uint8_t decay_midi;
        uint8_t volume_midi; // Volume of Track
        uint8_t pitch_midi;
        uint8_t offset_midi; // offset for samples
        uint8_t pan_midi;
        boolean is_muted;
    
        float pitchdecay = 0.0f;
        uint8_t pitchdecay_midi;
      
    } samplePlayerS ;
    
    samplePlayerS samplePlayer[ SAMPLECNT ];
    char filenames[ SAMPLECNT ][32];
   // samplePlayerS* samplePlayer = NULL;
    
    // float global_pitch_decay = 0.0f; // good from -0.2 to +1.0
    
    volatile int32_t sampleInfoCount = -1; // storing the count if found samples in file system 
    float slowRelease; // slow releasing signal will be used when sample playback stopped 
    uint8_t* RamCache = NULL ;

    FxFilterCrusher Effects;
};

inline void Sampler::SelectNote( uint8_t note ) {
      if(sampleInfoCount>0) selectedNote = note % repeat; else  selectedNote = note;
#ifdef DEBUG_SAMPLER
DEBF("Select note: %d\r\n", note);
#endif
};

inline void Sampler::Process( float *left, float *right ) {
  float signal_l = 0.0f;
  //signal_l += slowRelease;
  float signal_r = 0.0f;
  //signal_r += slowRelease;

  //slowRelease = slowRelease * 0.99; // go slowly to zero

  for ( int i = 0; i < sampleInfoCount; i++ ) {

    if ( samplePlayer[i].active  ) {
      samplePlayer[i].samplePos = samplePlayer[i].samplePosF;
      samplePlayer[i].samplePos -= samplePlayer[i].samplePos % 2;

      uint32_t dataOut = samplePlayer[i].samplePos;
      //  DEBUG(dataOut);

      //
      // reconstruct signal from data
      //
      uint8_t byte2 , byte1;
      union {
        uint16_t u16;
        int16_t s16;
      } sampleU;
      byte1 = RamCache[samplePlayer[i].sampleStart + dataOut];
      byte2 = RamCache[samplePlayer[i].sampleStart + dataOut + 1];
      sampleU.s16 = (((uint16_t)byte2) << 8U) + (uint16_t)byte1;

      samplePlayer[i].signal = (float)(samplePlayer[i].volume) * ((float)sampleU.s16) * 0.00005f;

      signal_l += samplePlayer[i].signal * samplePlayer[i].vel * ( 1 - samplePlayer[i].pan );

      signal_r += samplePlayer[i].signal * samplePlayer[i].vel *  samplePlayer[i].pan;

      samplePlayer[i].vel *= samplePlayer[i].decay;

      samplePlayer[i].samplePos += 2; // we have consumed two bytes

      if ( samplePlayer[i].pitchdecay > 0.0f ) {
        samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + samplePlayer[i].pitchdecay * samplePlayer[i].vel ); // we have consumed two bytes
      } else {
        samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + samplePlayer[i].pitchdecay * (1 - samplePlayer[i].vel) ); // we have consumed two bytes
      }

 //     samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch  ); // we have consumed two bytes
      if ( samplePlayer[i].samplePos >= samplePlayer[i].sampleSize ) {
        samplePlayer[i].active = false;
        
        samplePlayer[i].samplePos = 0;
        samplePlayer[i].samplePosF = 0.0f;
      }
    }
  }
  Effects.Process( &signal_l, &signal_r );
 // *left  = signal_l * _volume;
 // *right =  signal_r * _volume;
   *left  = General::fclamp(signal_l * _volume, -1.0f, 1.0f);
   *right = General::fclamp(signal_r * _volume, -1.0f, 1.0f);
  // *left  = General::fast_shape(signal_l * _volume);
  // *right = General::fast_shape(signal_r * _volume);
}

void Sampler::NoteOn( uint8_t note, uint8_t vol ) {

  /* check for null to avoid division by zero */
  if ( sampleInfoCount == 0 ) {
    return;
  }
  int j = note % sampleInfoCount;
  int param_i = note % repeat + 1;

  if ( is_muted[ param_i ] == true) {
    return;
  }

#ifdef GROUP_HATS
  switch (param_i) {
    case 7:
      samplePlayer[note+1].active = false;
      break;
    case 8:
      samplePlayer[note-1].active = false;
      break;
    default:
      break;
  }
#endif

#ifdef DEBUG_MIDI
  DEBF("note %d on volume %d\n", note, vol );
 // DEBF("Filename: %s \n", samplePlayer[ j ].filename );
#endif
  /*
    if( global_pitch_decay_midi != global_pitch_decay_midi_old ){
    global_pitch_decay_midi_old = global_pitch_decay_midi;
    if( global_pitch_decay_midi < 63 ){
      global_pitch_decay = (float) (65-global_pitch_decay_midi)/100; // good from -0.2 to +1.0
    }else if( global_pitch_decay_midi > 65 ){
      global_pitch_decay = (float) global_pitch_decay_midi/65; // good from -0.2 to +1.0
    }else{
      global_pitch_decay = 0.0f;
    }
    }
  */

  if ( volume_midi[ param_i ] != samplePlayer[ j ].volume_midi ) {
#ifdef DEBUG_MIDI
    DEB("Volume");
    DEBUG( j );
    DEB(" samplePlayer");
    DEBUG( volume_midi[ param_i ] );
#endif
    samplePlayer[ j ].volume_midi = volume_midi[ param_i ];
  }

  if ( decay_midi[ param_i ] != samplePlayer[ j ].decay_midi ) {
#ifdef DEBUG_MIDI
    DEB("Decay");
    DEBUG( j );
    DEB(" samplePlayer");
    DEBUG( decay_midi[ param_i ] );
#endif
    samplePlayer[ j ].decay_midi = decay_midi[ param_i ];
    float value = MIDI_NORM * decay_midi[ param_i ];
    samplePlayer[ j ].decay = 1 - (0.000005 * pow( 5000, 1.0f - value) );
  }

  if ( pitch_midi[ param_i ] != samplePlayer[ j ].pitch_midi ) {
#ifdef DEBUG_MIDI
    DEB("Pitch");
    DEBUG( j );
    DEB(" samplePlayer");
    DEBUG( pitch_midi[param_i ] );
#endif
    samplePlayer[ j ].pitch_midi = pitch_midi[ param_i ];
    float value = MIDI_NORM * pitch_midi[ param_i ];
    samplePlayer[ j ].pitch = pow( 2.0f, 4.0f * ( value - 0.5f ) );
  }

  if ( pan_midi[ param_i ] != samplePlayer[ j ].pan_midi ) {
#ifdef DEBUG_MIDI
    DEB("Pan");
    DEBUG( j );
    DEB(" samplePlayer");
    DEBUG( pan_midi[ param_i ] );
#endif
    samplePlayer[ j ].pan_midi = pan_midi[ param_i ];
    float value = MIDI_NORM * pan_midi[ param_i ];
    samplePlayer[ j ].pan = value;
  }

  if ( offset_midi[ param_i ] != samplePlayer[ j ].offset_midi ) {
#ifdef DEBUG_MIDI
    DEB("Attack Offset");
    DEBUG( j );
    DEB(" samplePlayer");
    DEBUG( offset_midi[ param_i ] );
#endif

    samplePlayer[ j ].offset_midi = offset_midi[ param_i ];
  }

  if ( pitchdecay_midi[ param_i ] != samplePlayer[ j ].pitchdecay_midi ) {

    samplePlayer[ j ].pitchdecay_midi = pitchdecay_midi[ param_i ];
    samplePlayer[ j ].pitchdecay = 0.0f; // default
    if ( samplePlayer[ j ].pitchdecay_midi < 63 ) {
      samplePlayer[ j ].pitchdecay = (float) (63 - samplePlayer[ j ].pitchdecay_midi ) / 20.0f; // good from -0.2 to +1.0
    } else if ( samplePlayer[ j ].pitchdecay_midi > 65 ) {
      samplePlayer[ j ].pitchdecay = (float) - ( samplePlayer[ j ].pitchdecay_midi - 65) / 30.0f; // good from -0.2 to +1.0
    }
#ifdef DEBUG_MIDI
    DEB("PitchDecay");
    DEBUG( j );
    DEB(" samplePlayer ");
    DEB( pitchdecay_midi[ param_i ] );
    DEB(" FloatValue: " );
    DEBUG( samplePlayer[ j ].pitchdecay );
#endif
  }


  samplePlayerS *newSamplePlayer = &samplePlayer[j];

  if ( newSamplePlayer->active ) {
    /* add last output signal to slow release to avoid noise */
    slowRelease = newSamplePlayer->signal;
  }

  newSamplePlayer->samplePosF = 4.0f * newSamplePlayer->offset_midi; // 0.0f;
  newSamplePlayer->samplePos  = 4 * newSamplePlayer->offset_midi; // 0;

  newSamplePlayer->volume = vol * MIDI_NORM * newSamplePlayer->volume_midi * MIDI_NORM;
  newSamplePlayer->vel    = 1.0f;
 // newSamplePlayer->dataIn = 0;
  newSamplePlayer->sampleSeek = 44 + 4 * newSamplePlayer->offset_midi; // 16 Bit-Samples wee nee

  newSamplePlayer->active = true;
}

void Sampler::NoteOff( uint8_t note ) {
  /*
     nothing to do yet
     we could stop samples if we want to
  */
  if ( sampleInfoCount == 0 ) {
    return;
  }
  // int j = note % sampleInfoCount;
  // samplePlayer[j]->active = false;
}

inline void Sampler::PitchBend(int number) {
  //-8192 to 8191, 0 = original pitch
}

inline void Sampler::ParseCC(uint8_t cc_number , uint8_t cc_value) {
  switch (cc_number) {
    case CC_808_VOLUME:
      SetVolume( cc_value * MIDI_NORM );
      break;
    case CC_808_NOTE_PAN:
      SetNotePan_Midi( cc_value );
      break;
    case CC_808_RESO:
      Effects.SetResonance( cc_value * MIDI_NORM );
      break;
    case CC_808_CUTOFF:
      Effects.SetCutoff( cc_value * MIDI_NORM );
      break;
    case CC_808_NOTE_ATTACK:
      SetNoteOffset_Midi( cc_value );
      break;
    case CC_808_NOTE_DECAY:
      SetNoteDecay_Midi( cc_value );
      break;
    case CC_808_PITCH:
      SetSoundPitch_Midi ( cc_value );
      break;
    case CC_808_DELAY_SEND:
      _sendDelay = cc_value * MIDI_NORM;
      break;
    case CC_808_REVERB_SEND:
      _sendReverb = cc_value * MIDI_NORM;
      break;
    case CC_808_DISTORTION:
      if ( cc_value == 0 ) Effects.SetBitCrusher(0.0f);
      else Effects.SetBitCrusher( 0.66f + (cc_value * MIDI_NORM * 0.23f) );
      break;
    case CC_808_NOTE_SEL:
      SelectNote( cc_value );
      break;
    case CC_808_BD_DECAY:
      SelectNote( 0 ); // BD
      SetNoteDecay_Midi( cc_value );
      break;
    case CC_808_BD_TONE:
      SelectNote( 0 ); // BD
      SetSoundPitch_Midi ( cc_value );
      break;
    case CC_808_BD_LEVEL:
      SelectNote( 0 ); // BD
      SetNoteVolume_Midi ( cc_value );
      break;
    case CC_808_SD_SNAP:
      SelectNote( 1 ); // SD
      SetNoteDecay_Midi( cc_value );
      break;
    case CC_808_SD_TONE:
      SelectNote( 1 ); // SD
      SetSoundPitch_Midi( cc_value );
      break;
    case CC_808_SD_LEVEL:
      SelectNote( 1 ); // SD
      SetNoteVolume_Midi( cc_value );
      break;
    case CC_808_CH_TUNE:
      SelectNote( 6 ); // CH
      SetSoundPitch_Midi( cc_value );
      break;
    case CC_808_CH_LEVEL:
      SelectNote( 6 ); // CH
      SetNoteVolume_Midi( cc_value );
      break;
    case CC_808_OH_TUNE:
      SelectNote( 7 ); // OH
      SetSoundPitch_Midi( cc_value );
      break;
    case CC_808_OH_LEVEL:
      SelectNote( 7 ); // OH
      SetNoteVolume_Midi( cc_value );
      break;
    case CC_808_OH_DECAY:
      SelectNote( 7 ); // OH
      SetNoteDecay_Midi( cc_value );
      break;
      /*
        #define CC_808_BD_TONE    21  // Specific per drum control
        #define CC_808_BD_DECAY   23
        #define CC_808_BD_LEVEL   24
        #define CC_808_SD_TONE    25
        #define CC_808_SD_SNAP    26
        #define CC_808_SD_LEVEL   29
        #define CC_808_CH_TUNE    61
        #define CC_808_CH_LEVEL   63
        #define CC_808_OH_TUNE    80
        #define CC_808_OH_DECAY   81
        #define CC_808_OH_LEVEL   82
      */
  }
}

inline void Sampler::SetNotePan_Midi( uint8_t data1) {
  /*
    samplePlayer[ selectedNote ].pan_midi = data1;
    float value = MIDI_NORM * (float)data1;
    samplePlayer[ selectedNote ].pan =  value;
    #ifdef DEBUG_SAMPLER
    DEBF("Sampler - Note[%d].pan: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].pan );
    #endif
  */
  pan_midi[ selectedNote + 1 ] = data1;
#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d].midi_pan: %d\n",  selectedNote, data1 );
#endif
}

inline void Sampler::SetNoteOffset_Midi( uint8_t data1) {
  /*
    samplePlayer[ selectedNote ].offset_midi = data1;
    #ifdef DEBUG_SAMPLER
    DEBF("Sampler - Note[%d].offset: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].offset_midi);
    #endif
  */

#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d].offset_midi: %d\n",  selectedNote, data1);
#endif
  offset_midi[ selectedNote + 1 ] = data1;
}

inline void Sampler::SetNoteDecay_Midi( uint8_t data1) {
  /*
    samplePlayer[ selectedNote ].decay_midi = data1;
    float value = MIDI_NORM * (float)data1;
    // samplePlayer[ selectedNote ].decay = 1.0f - (0.000005f * pow( 5000.0f, 1.0f - value) );
    samplePlayer[ selectedNote ].decay = 1.0f -  value * 0.05 ;
    #ifdef DEBUG_SAMPLER
    DEBF("Sampler - Note[%d].decay: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].decay);
    #endif
  */

#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d].decay_midi: %d\n",  selectedNote, data1);
#endif
  decay_midi[ selectedNote + 1 ] = data1;
}

inline void Sampler::SetNoteVolume_Midi(uint8_t data1) {
  volume_midi[ selectedNote + 1 ] = data1;
#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d].midi_vol: %d\n",  selectedNote, data1 );
#endif
}

inline void Sampler::SetSoundPitch_Midi( uint8_t data1) {
  /*
    samplePlayer[ selectedNote ].pitch_midi = data1;
    SetSoundPitch( MIDI_NORM * data1 );
  */
#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d].pitch_midi: %d\n",  selectedNote, data1);
#endif
  pitch_midi[ selectedNote + 1 ] = data1;
}

inline void Sampler::SetSoundPitch(float value) {
  samplePlayer[ selectedNote ].pitch = pow( 2.0f, 4.0f * ( value - 0.5f ) );
#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d] pitch: %0.3f\n",  selectedNote, samplePlayer[ selectedNote ].pitch );
#endif
}

inline void Sampler::SetDelaySend(uint8_t lvl) {
  _sendDelay = (float)lvl;
};

inline void Sampler::SetReverbSend(uint8_t lvl) {
  _sendReverb = (float)lvl;
}

#endif
