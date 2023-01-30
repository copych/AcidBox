/*
 * this file includes the implementation of the sample player
 * all samples are loaded from LittleFS stored on the external flash
 *
 * Author: Marcel Licence
 * 
 * Modifications:
 * 2021-04-05 E.Heinemann changed BLOCKSIZE from 2024 to 1024
 *             , added DEBUG_SAMPLER
 *             , added sampleRate to the structure of samplePlayerS to optimize the pitch based on lower samplerates
 * 2021-07-28 E.Heinemann, added pitch-decay and pan
 * 2021-08-03 E.Heinemann, changed Accent/normal Velocity in the code
 * 2022-11-27 Copych, made this a class, made it use one big PSRAM buffer for wave data
 */
 
#include "sampler.h"

/* You only need to format LittleFS the first time you run a
   test or else use the LittleFS plugin to create a partition
   https://github.com/lorol/arduino-esp32LittleFS-plugin */

//#define DEBUG_SAMPLER

void Sampler::ScanContents(fs::FS &fs, const char *dirname, uint8_t levels){
#ifdef DEBUG_SAMPLER  
    DEBF("Listing directory: %s\r\n", dirname);
#endif
    File root = fs.open(dirname);
    if( !root ){
        DEBUG("- failed to open directory");
        return;
    }
    if( !root.isDirectory() ){
        DEBUG(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while( file ){
        if( file.isDirectory() ){
#ifdef DEBUG_SAMPLER
            DEB("  DIR : ");
            DEBUG(file.name());
#endif            
            if( levels ){
                
                ScanContents(fs, file.name(), levels - 1);
            }
        }else{
#ifdef DEBUG_SAMPLER          
            DEB("  FILE: ");
            DEB(dirname);
            DEB(file.name());
            DEB("\tSIZE: ");
            DEBUG(file.size());
#endif
            String sDir = String(dirname);
            String fname = String(file.name());  
            String fullName = sDir + fname;
            if( sampleInfoCount < SAMPLECNT ){
                strncpy( samplePlayer[ sampleInfoCount ].filename, fullName.c_str() , 32);
                sampleInfoCount ++;              
                shortInstr[ sampleInfoCount ] = fname.substring(fname.length()-7, fname.length()-4);
            }
        }
        delay(1);
        file = root.openNextFile();
    }
}


inline void Sampler::Init(){
    Effects.Init();
    Effects.SetBitCrusher( 0.0f );
    
    uint32_t toRead = 512, buffPointer = 0;

    if( !LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)){
        DEBUG("LittleFS Mount Failed");
        return;
    }
    
    String myDir = "/" + (String)progNumber + "/";
    
    sampleInfoCount = 0;
    ScanContents(LittleFS, myDir.c_str() , 5);

#ifndef NO_PSRAM
    // allocate ~1MB buffer in PSRAM to be able to load a whole sample set of SAMPLECNT {12} samples
    if (psramFound()) {
      if ( RamCache == NULL ) {
        psramInit();
        RamCache = (uint8_t*)ps_malloc(PSRAM_SAMPLER_CACHE);
      }   
      if (RamCache == NULL){
        DEBUG ("FAILED TO ALLOCATE PSRAM CACHE BUFFER!");
      } else {
        DEBUG ("PSRAM BUFFER ALLOCATED! STANDARD CONFIG ENGAGED!");
      }
    } else {
      DEBUG("STOP! Use #define NO_PSRAM option in config.h");
      while(1){}
    }
#else
      DEBF("Free heap: %d\r\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
      heap_caps_print_heap_info(MALLOC_CAP_8BIT);
      if ( RamCache==NULL ) {
        RamCache = (uint8_t*)malloc(RAM_SAMPLER_CACHE); 
       // RamCache = (uint8_t*)heap_caps_malloc(RAM_SAMPLER_CACHE, MALLOC_CAP_8BIT);
      }
      if (RamCache == NULL){
        DEBUG ("FAILED TO ALLOCATE RAM CACHE BUFFER!");
      } else {
        DEBUG ("HEAP BUFFER ALLOCATED! MINIMAL CONFIG ENGAGED!");
      }

#endif

#ifdef DEBUG_SAMPLER
    DEBUG("---\nListSamples:");
#endif
    for (int i = 0; i < sampleInfoCount; i++ ){
#ifdef DEBUG_SAMPLER      
        DEBF( "s[%d]: %s\n", i, samplePlayer[i].filename );
#endif 

        File f = LittleFS.open( String( samplePlayer[i].filename) );

        if( f ){
            union wavHeader wav;
            int j = 0;
            while( f.available() && ( j < sizeof(wav.wavHdr)) ){
                wav.wavHdr[j] = f.read();
                j++;
            }

            j = 0;
            // load sample data to the RAM/PSRAM buffer
            samplePlayer[i].sampleStart = buffPointer;
            while( f.available() ){
                toRead = 512;
                f.read(&(RamCache[buffPointer]), toRead);
                buffPointer += toRead;
            }

            samplePlayer[i].file = f; /* store file pointer for future use */
            samplePlayer[i].sampleRate = wav.sampleRate;
#ifdef DEBUG_SAMPLER
            DEBF("fileSize: %d\n",      wav.fileSize);
            DEBF("lengthOfData: %d\n",  wav.lengthOfData);
            DEBF("numberOfChannels: %d\n", wav.numberOfChannels);
            DEBF("sampleRate: %d\n",    wav.sampleRate);
            DEBF("byteRate: %d\n",      wav.byteRate);
            DEBF("bytesPerSample: %d\n", wav.bytesPerSample);
            DEBF("bitsPerSample: %d\n", wav.bitsPerSample);
            DEBF("dataSize: %d\n",      wav.dataSize);
            DEBF("dataStartinBuffer: %d\n",    samplePlayer[i].sampleStart);
            DEBF("dataInBlock: %d\n",   (buffPointer-samplePlayer[i].sampleStart));
#endif            
            samplePlayer[i].sampleSize =         wav.dataSize; /* without mark section and size info */
            samplePlayer[i].sampleSeek = 0xFFFFFFFF;
        }else{
            DEBF("error openening file!\n");
        }
    }

    for( int i = 0; i < SAMPLECNT; i++ ){

        samplePlayer[i].sampleSeek = 0xFFFFFFFF;
        samplePlayer[i].active = false;

        decay_midi[i+1] = 100;
        samplePlayer[i].decay_midi = decay_midi[i+1];
        samplePlayer[i].decay = 1.0f;

        offset_midi[i+1] = 0;
        samplePlayer[i].offset_midi = offset_midi[i+1];
        
        volume_midi[i+1] = 100;
        samplePlayer[i].volume_midi = volume_midi[i+1];

        pan_midi[i+1] = 64;
        samplePlayer[i].pan_midi = pan_midi[i+1];
        samplePlayer[i].pan = 0.5;

        pitch_midi[i+1] = 64;
        samplePlayer[i].pitch_midi = pitch_midi[i+1];
        if( samplePlayer[i].sampleRate > 0 ){
          samplePlayer[i].pitch = 1.0f / SAMPLE_RATE * samplePlayer[i].sampleRate;
        } 
    };

}

inline void Sampler::SetNotePan_Midi( uint8_t data1){
  /*
  samplePlayer[ selectedNote ].pan_midi = data1;
  float value = MIDI_NORM * (float)data1;
  samplePlayer[ selectedNote ].pan =  value;
#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].pan: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].pan );
#endif  
*/
  pan_midi[ selectedNote+1 ] = data1;
#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].midi_pan: %0.2f\n",  selectedNote, data1 );
#endif 
}


inline void Sampler::SetNoteDecay_Midi( uint8_t data1){
  /*
  samplePlayer[ selectedNote ].decay_midi = data1;
  float value = MIDI_NORM * (float)data1;
 // samplePlayer[ selectedNote ].decay = 1.0f - (0.000005f * pow( 5000.0f, 1.0f - value) );
  samplePlayer[ selectedNote ].decay = 1.0f -  value * 0.05 ;
#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].decay: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].decay);
#endif  
*/

#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].decay_midi: %0.2f\n",  selectedNote, data1);
#endif 
  decay_midi[ selectedNote+1 ] = data1;
}


inline void Sampler::SetNoteOffset_Midi( uint8_t data1){
  /*
  samplePlayer[ selectedNote ].offset_midi = data1;   
#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].offset: %0.2f\n",  selectedNote, samplePlayer[ selectedNote ].offset_midi);
#endif  
*/

#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].offset_midi: %0.2f\n",  selectedNote, data1);
#endif 
  offset_midi[ selectedNote+1 ] = data1;
}

inline void Sampler::SetSoundPitch_Midi( uint8_t data1){
/*  
  samplePlayer[ selectedNote ].pitch_midi = data1;
  SetSoundPitch( MIDI_NORM * data1 );
*/
#ifdef DEBUG_SAMPLER
  DEBF("Sampler - Note[%d].pitch_midi: %0.2f\n",  selectedNote, data1);
#endif 
  pitch_midi[ selectedNote+1 ] = data1;
}

inline void Sampler::SetSoundPitch(float value){
  samplePlayer[ selectedNote ].pitch = pow( 2.0f, 4.0f * ( value - 0.5f ) );
#ifdef DEBUG_SAMPLER  
  DEBF("Sampler - Note[%d] pitch: %0.3f\n",  selectedNote, samplePlayer[ selectedNote ].pitch );
#endif 
}


inline void Sampler::NoteOn( uint8_t note, uint8_t vol ){
  
    /* check for null to avoid division by zero */
    if( sampleInfoCount == 0 ){
        return;
    }
    int j = note % sampleInfoCount;

    if( is_muted[ j+1 ]== true){
      return;
    }

#ifdef DEBUG_SAMPLER
    DEBF("note %d on volume %d\n", note, vol );
    DEBF("Filename: %s \n", samplePlayer[ j ].filename );    
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

    if( volume_midi[ j+1 ] != samplePlayer[ j ].volume_midi ){
#ifdef DEBUG_SAMPLER
      DEB("Volume");
      DEBUG( j );      
      DEB(" samplePlayer");
      DEBUG( volume_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].volume_midi = volume_midi[ j+1 ];
    }

    if( decay_midi[ j+1 ] != samplePlayer[ j ].decay_midi ){
#ifdef DEBUG_SAMPLER
      DEB("Decay");
      DEBUG( j );      
      DEB(" samplePlayer");
      DEBUG( decay_midi[ j+1 ] );
#endif     
      samplePlayer[ j ].decay_midi = decay_midi[ j+1 ];
      float value = MIDI_NORM * decay_midi[ j+1 ];
      samplePlayer[ j ].decay = 1 - (0.000005 * pow( 5000, 1.0f - value) );
    }

    if( pitch_midi[ j+1 ] != samplePlayer[ j ].pitch_midi ){
#ifdef DEBUG_SAMPLER
      DEB("Pitch");
      DEBUG( j );
      DEB(" samplePlayer");
      DEBUG( pitch_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].pitch_midi = pitch_midi[ j+1 ];
      float value = MIDI_NORM * pitch_midi[ j+1 ];
      samplePlayer[ j ].pitch = pow( 2.0f, 4.0f * ( value - 0.5f ) );
    }

    if( pan_midi[ j+1 ] != samplePlayer[ j ].pan_midi ){
#ifdef DEBUG_SAMPLER
      DEB("Pan");
      DEBUG( j );      
      DEB(" samplePlayer");
      DEBUG( pan_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].pan_midi = pan_midi[ j+1 ];
      float value = MIDI_NORM * pan_midi[ j+1 ];
      samplePlayer[ j ].pan = value;
    }

    if( offset_midi[ j+1 ] != samplePlayer[ j ].offset_midi ){
#ifdef DEBUG_SAMPLER
      DEB("Attack Offset");
      DEBUG( j );      
      DEB(" samplePlayer");
      DEBUG( offset_midi[ j+1 ] );
#endif   
      samplePlayer[ j ].offset_midi = offset_midi[ j+1 ];
    }
    
    if( pitchdecay_midi[ j+1 ] != samplePlayer[ j ].pitchdecay_midi ){
 
      samplePlayer[ j ].pitchdecay_midi = pitchdecay_midi[ j+1 ];
      samplePlayer[ j ].pitchdecay = 0.0f; // default
      if( samplePlayer[ j ].pitchdecay_midi < 63 ){ 
        samplePlayer[ j ].pitchdecay = (float) (63 - samplePlayer[ j ].pitchdecay_midi ) /20.0f; // good from -0.2 to +1.0
      }else if( samplePlayer[ j ].pitchdecay_midi > 65 ){ 
        samplePlayer[ j ].pitchdecay = (float) -( samplePlayer[ j ].pitchdecay_midi -65) /30.0f; // good from -0.2 to +1.0
      }
#ifdef DEBUG_SAMPLER
      DEB("PitchDecay");
      DEBUG( j );      
      DEB(" samplePlayer ");
      DEB( pitchdecay_midi[ j+1 ] );
      DEB(" FloatValue: " );
      DEBUG( samplePlayer[ j ].pitchdecay );
#endif  
    }


    struct samplePlayerS *newSamplePlayer = &samplePlayer[j];
    
    if( newSamplePlayer->active ){
        /* add last output signal to slow release to avoid noise */
        slowRelease = newSamplePlayer->signal;
    }

    newSamplePlayer->samplePosF = 4.0f * newSamplePlayer->offset_midi; // 0.0f;
    newSamplePlayer->samplePos  = 4 * newSamplePlayer->offset_midi; // 0;
 //   newSamplePlayer->lastDataOut = PRELOADSIZE; /* trigger loading second half */
    
    newSamplePlayer->volume = vol * MIDI_NORM * newSamplePlayer->volume_midi * MIDI_NORM;
    newSamplePlayer->vel    = 1.0f;
    newSamplePlayer->dataIn = 0;
    newSamplePlayer->sampleSeek = 44 + 4*newSamplePlayer->offset_midi; // 16 Bit-Samples wee nee

    newSamplePlayer->active = true;

}

inline void Sampler::NoteOff( uint8_t note ){
    /*
     * nothing to do yet
     * we could stop samples if we want to
     */
       if( sampleInfoCount == 0 ){
        return;
    }
   // int j = note % sampleInfoCount;
    // samplePlayer[j]->active = false;
}

void Sampler::SetPlaybackSpeed( float value ){
    value = pow( 2.0f, 4.0f * (value - 0.5) );
    DEBF( "SetPlaybackSpeed: %0.2f\n", value );
    sampler_playback = value;
}

void Sampler::SetProgram( uint8_t prog ){
  progNumber = prog % countPrograms;
  Init();
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
    case CC_808_SD_SNAP:
      SelectNote( 1 ); // SD
      SetNoteDecay_Midi( cc_value );
      break;
    case CC_808_SD_TONE:
      SelectNote( 1 ); // SD
      SetSoundPitch_Midi( cc_value );
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


inline void Sampler::Process( float *left, float *right ){

    if( progNumber !=  program_tmp ){
      SetProgram( program_tmp );
    }
  
    float signal_l = 0.0f;
    signal_l += slowRelease;
    float signal_r = 0.0f;
    signal_r += slowRelease;
    
    slowRelease = slowRelease * 0.99; // go slowly to zero 

    for( int i = 0; i < SAMPLECNT; i++ ){

        if( samplePlayer[i].active  ){
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

            signal_l += samplePlayer[i].signal * samplePlayer[i].vel * ( 1- samplePlayer[i].pan );

            signal_r += samplePlayer[i].signal * samplePlayer[i].vel *  samplePlayer[i].pan;

            samplePlayer[i].vel *= samplePlayer[i].decay;

            samplePlayer[i].samplePos += 2; // we have consumed two bytes 

            if ( samplePlayer[i].pitchdecay > 0.0f ) {
              samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + samplePlayer[i].pitchdecay*samplePlayer[i].vel ); // we have consumed two bytes 
            } else {
              samplePlayer[i].samplePosF += 2.0f * sampler_playback * ( samplePlayer[i].pitch + samplePlayer[i].pitchdecay*(1-samplePlayer[i].vel) ); // we have consumed two bytes 
            } 

            if( samplePlayer[i].samplePos >= samplePlayer[i].sampleSize ){
              samplePlayer[i].active = false;
              samplePlayer[i].samplePos= 0;
              samplePlayer[i].samplePosF= 0.0f;
            }
        }
    }
    Effects.Process( &signal_l, &signal_r );
    signal_l *= _volume;
    signal_r *= _volume;
  //  *left  = fclamp(signal_l, -1.0f, 1.0f);
  //  *right = fclamp(signal_r, -1.0f, 1.0f);
    *left  = fast_tanh(signal_l);
    *right = fast_tanh(signal_r);
}
