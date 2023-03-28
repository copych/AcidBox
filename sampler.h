#ifndef SAMPLER_H
#define SAMPLER_H

#include <FS.h>
#include <LittleFS.h>
#include "midi_config.h"
#include "fx_filtercrusher.h"

class Sampler {
  public:
    Sampler(){}
    Sampler(uint8_t progNow) { program_tmp = progNow; progNumber = progNow; };
    void Init();
    void ScanContents(fs::FS &fs, const char *dirname, uint8_t levels);
    inline void SelectNote( uint8_t note ){
      if(sampleInfoCount>0) selectedNote = note % repeat; else  selectedNote = note;
#ifdef DEBUG_SAMPLER
DEBF("Select note: %d\r\n", note);
#endif
    };
    inline void SetNotePan_Midi( uint8_t data1);
    inline void SetNoteOffset_Midi( uint8_t data1 );
    inline void SetNoteDecay_Midi( uint8_t data1); 
    inline void SetNoteVolume_Midi( uint8_t data1);
    inline void SetSoundPitch_Midi( uint8_t data1);
    inline void SetSoundPitch(float value);   
    inline void SetDelaySend(uint8_t lvl)   {_sendDelay = (float)lvl;};
    inline void SetReverbSend(uint8_t lvl)  {_sendReverb = (float)lvl;};
    uint16_t GetSoundSamplerate() { return samplePlayer[ selectedNote ].sampleRate; };
    uint8_t GetSoundDecay_Midi()  { return samplePlayer[ selectedNote ].decay_midi; };
    uint16_t GetSoundPan_Midi()   { return samplePlayer[ selectedNote ].pan_midi; };
    uint8_t GetSoundPitch_Midi()  { return samplePlayer[ selectedNote ].pitch_midi; };
    uint8_t GetSoundVolume_Midi() { return samplePlayer[ selectedNote ].volume_midi; };
    int32_t GetSamplesCount()     { return sampleInfoCount; }
    // Offset   for the Sample-Playback to cut the sample from the left
    inline void NoteOn( uint8_t note, uint8_t vol );
    inline void NoteOff( uint8_t note );
    void SetPlaybackSpeed_Midi( uint8_t value ){  SetSoundPitch( (float) MIDI_NORM * value ); };
    void SetPlaybackSpeed( float value );
    void SetProgram( uint8_t prog );
    void SetVolume( float value ) { _volume = value; };
    inline void Process( float *left, float *right );
    inline void ParseCC(uint8_t cc_number, uint8_t cc_value);
    inline void PitchBend(int number);
    float _sendReverb = 0.0f;
    float _sendDelay = 0.0f;
    
  private:
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

#endif
