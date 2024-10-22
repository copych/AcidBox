/*
   this file includes the implementation of the sample player
   all samples are loaded from LittleFS stored on the external flash

   Author: Marcel Licence

   Modifications:
   2021-04-05 E.Heinemann changed BLOCKSIZE from 2024 to 1024
               , added DEBUG_SAMPLER
               , added sampleRate to the structure of samplePlayerS to optimize the pitch based on lower samplerates
   2021-07-28 E.Heinemann, added pitch-decay and pan
   2021-08-03 E.Heinemann, changed Accent/normal Velocity in the code
   2022-11-27 Copych, made this a class, made it use one big PSRAM buffer for drumkit wav data
   2023-01-20 Copych, changed midi cc handling, now it affects instruments, not the sample players
   2023-03-02 Copych, preload all 3MB of samples from flash to PSRAM to be able of switching kits in realtime
*/

#include "sampler.h"
#include "samples.h"

/* You only need to format LittleFS the first time you run a
   test or else use the LittleFS plugin to create a partition
   https://github.com/lorol/arduino-esp32LittleFS-plugin */

//#define DEBUG_SAMPLER
void Sampler::CreateDefaultSamples(fs::FS &fs){
  const String path = "/0";
  size_t toWrite = 0;
  fs.mkdir(path);
  WriteFile(fs, (String)(path + "/001_BD.wav"), s01_sz, s01);
  WriteFile(fs, (String)(path + "/002_SD.wav"), s02_sz, s02);
  WriteFile(fs, (String)(path + "/003_.wav"), s00_sz, s00);
  WriteFile(fs, (String)(path + "/004_.wav"), s00_sz, s00);
  WriteFile(fs, (String)(path + "/005_CB.wav"), s05_sz, s05);
  WriteFile(fs, (String)(path + "/006_.wav"), s00_sz, s00);
  WriteFile(fs, (String)(path + "/007_CH.wav"), s07_sz, s07);
  WriteFile(fs, (String)(path + "/008_OH.wav"), s08_sz, s08);
  WriteFile(fs, (String)(path + "/009_.wav"), s00_sz, s00);
  WriteFile(fs, (String)(path + "/010_CR.wav"), s10_sz, s10);
}

void Sampler::WriteFile(fs::FS &fs, const String fname, size_t fsize, const uint8_t bytearray[] ) {
  size_t len = fsize;
  size_t toWrite = len;
  size_t arrPointer = 0;
  File f = fs.open(fname, FILE_WRITE);
  while( len ){
    if(len > (1UL<<15)){
      toWrite = (1UL<<15);
    } else {
      toWrite = len;
    }
    f.write(&(bytearray[arrPointer]), toWrite);
    arrPointer += toWrite;
    len -= toWrite;
  }
  DEBF("[sampler]: %s written %d bytes to flash\r\n", fname, fsize);
  f.close();
}

void Sampler::ScanContents(fs::FS &fs, const char *dirname, uint8_t levels) {
  String str;
#ifdef DEBUG_SAMPLER
  DEBF("Listing directory: %s\r\n", dirname);
#endif
  File root = fs.open(dirname);
  if ( !root ) {
    DEBUG("- failed to open directory");
    return;
  }
  if ( !root.isDirectory() ) {
    DEBUG(" - not a directory");
    return;
  }

  File file = root.openNextFile();
  while ( file ) {
    if ( file.isDirectory() ) {
#ifdef DEBUG_SAMPLER
      DEB("  DIR : ");
      DEBUG(file.name());
#endif
      if ( levels ) {
        str = (String)(dirname + (String)(file.name()) + '/');
        ScanContents(fs, str.c_str(), levels - 1);
      }
    } else {
#ifdef DEBUG_SAMPLER
      DEB("  FILE: ");
      DEB(dirname);
      DEB(file.name());
      DEB("\tSIZE: ");
      DEBUG(file.size());
#endif

      if ( sampleInfoCount < SAMPLECNT ) {
        str = (String)(file.name());
       // shortInstr[ sampleInfoCount ] = str.substring(str.length() - 7, str.length() - 4);
        str = (String)dirname + str;
//        strncpy( samplePlayer[ sampleInfoCount ].filename, str.c_str() , 32);
        strncpy( filenames[ sampleInfoCount ], str.c_str() , 32);
        sampleInfoCount ++;
      }
    }
    delay(1);
    file = root.openNextFile();
  }
}


void Sampler::Init() {
 // samplePlayer = (samplePlayerS*)heap_caps_malloc( SAMPLECNT * sizeof( *samplePlayer), MALLOC_CAP_8BIT);
  
  Effects.Init();
  Effects.SetBitCrusher( 0.0f );

  size_t toRead = 512, oldPointer = 0, buffPointer = 0;

  if ( !LittleFS.begin(FORMAT_LITTLEFS_IF_FAILED)) {
    DEBUG("LittleFS Mount Failed");
    return;
  }
#ifdef NO_PSRAM
  String myDir = "/" + (String)progNumber + "/";
#else
  #ifdef PRELOAD_ALL
    String myDir = "/" ;
  #else
    String myDir = "/" + (String)progNumber + "/";
  #endif
#endif

  sampleInfoCount = 0;
  ScanContents(LittleFS, myDir.c_str() , 5);
  if (sampleInfoCount<5) {
    CreateDefaultSamples(LittleFS);
    ScanContents(LittleFS, myDir.c_str() , 5);
  }
  repeat = min(sampleInfoCount , repeat); // 12 (an octave) or less

  if (repeat==0) repeat = 1;
  
#ifndef NO_PSRAM
  // allocate buffer in PSRAM to be able to load all samples 
  heap_caps_print_heap_info(MALLOC_CAP_8BIT);
  if (psramFound()) {
    if ( RamCache == NULL ) {
      psramInit();
      RamCache = (uint8_t*)ps_malloc(PSRAM_SAMPLER_CACHE);
     // RamCache = (uint8_t*)malloc(PSRAM_SAMPLER_CACHE);
    }
    if (RamCache == NULL) {
      DEBUG ("FAILED TO ALLOCATE PSRAM CACHE BUFFER!");
    } else {
      DEBF ("PSRAM BUFFER OF %d bytes ALLOCATED! STANDARD CONFIG ENGAGED!\r\n", PSRAM_SAMPLER_CACHE );
      
      heap_caps_print_heap_info(MALLOC_CAP_8BIT);
    }
  } else {
    DEBUG("STOP! Use #define NO_PSRAM option in config.h");
    while (1) {}
  }
#else
  DEBF("Free heap: %d\r\n", heap_caps_get_free_size(MALLOC_CAP_8BIT));
  heap_caps_print_heap_info(MALLOC_CAP_8BIT);
  if ( RamCache == NULL ) {
    RamCache = (uint8_t*)malloc(RAM_SAMPLER_CACHE);
    // RamCache = (uint8_t*)heap_caps_malloc(RAM_SAMPLER_CACHE, MALLOC_CAP_8BIT);
  }
  if (RamCache == NULL) {
    DEBUG ("FAILED TO ALLOCATE RAM CACHE BUFFER!");
  } else {
    DEBF ("HEAP BUFFER of %d bytes ALLOCATED! MINIMAL CONFIG ENGAGED!\r\n", RAM_SAMPLER_CACHE);
  }

#endif
#ifdef DEBUG_SAMPLER
  DEBUG("---\nList Samples:");
#endif
  for (int i = 0; i < sampleInfoCount; i++ ) {
#ifdef DEBUG_SAMPLER
//    DEBF( "s[%d]: %s\n", i, samplePlayer[i].filename );
    DEBF( "s[%d]: %s\n", i, filenames[i] );
#endif

//    File f = LittleFS.open( (String)(samplePlayer[i].filename) );
    File f = LittleFS.open( (String)(filenames[i]) );

    if ( f ) {
      size_t len = f.size();
      union wavHeader wav;
      if ( len ) {
        toRead = sizeof(wav.wavHdr);
        f.read(&(wav.wavHdr[0]),toRead);
        len -= toRead;
      }

      // load sample data to the RAM/PSRAM buffer, we only do this step on startup
      samplePlayer[i].sampleStart = buffPointer;
      oldPointer = buffPointer;
      while( len ){
        if(len > (1UL<<15)){
          toRead = (1UL<<15);
        } else {
          toRead = len;
        }
        f.read(&(RamCache[buffPointer]), toRead);
        buffPointer += toRead;
        len -= toRead;
      }
      wav.dataSize = min(wav.dataSize, buffPointer-oldPointer); // some samples have wrong header info
      
  //    samplePlayer[i].file =            f;// store file pointer for future use // nope, we don't, we close file, LittleFS won't let us keep so many open files, neither  memory...
      samplePlayer[i].sampleRate =      wav.sampleRate;
#ifdef DEBUG_SAMPLER
      DEBF("fileSize: %d\n",            wav.fileSize);
      DEBF("lengthOfData: %d\n",        wav.lengthOfData);
      DEBF("numberOfChannels: %d\n",    wav.numberOfChannels);
      DEBF("sampleRate: %d\n",          wav.sampleRate);
//      DEBF("byteRate: %d\n",            wav.byteRate);
//      DEBF("bytesPerSample: %d\n",      wav.bytesPerSample);
      DEBF("bitsPerSample: %d\n",       wav.bitsPerSample);
      DEBF("dataSize: %d\n",            wav.dataSize); 
//      DEBF("dataStartInBuffer: %d\n",   samplePlayer[i].sampleStart);
//      DEBF("dataInBlock: %d\n",         (buffPointer - samplePlayer[i].sampleStart));
#endif
      samplePlayer[i].sampleSize =      wav.dataSize; /* without mark section and size info */
      samplePlayer[i].sampleSeek =      0xFFFFFFFF;
      f.close();
    } else {
      DEBF("error opening file!\n");
    }
  }

  for ( int i = 0; i < sampleInfoCount; i++ ) {
    int j = (i % repeat ) + 1 ; 
    samplePlayer[i].sampleSeek = 0xFFFFFFFF;
    samplePlayer[i].active = false;

    decay_midi[j] = 100;
    samplePlayer[i].decay_midi = decay_midi[j];
    samplePlayer[i].decay = 1.0f;

    offset_midi[j] = 0;
    samplePlayer[i].offset_midi = offset_midi[j];

    volume_midi[j] = 100;
    samplePlayer[i].volume_midi = volume_midi[j];

    pan_midi[j] = 64;
    samplePlayer[i].pan_midi = pan_midi[j];
    samplePlayer[i].pan = 0.5;

    pitch_midi[j] = 64;
    samplePlayer[i].pitch_midi = pitch_midi[j];
    if ( samplePlayer[i].sampleRate > 0 ) {
      samplePlayer[i].pitch = 1.0f / SAMPLE_RATE * samplePlayer[i].sampleRate;
    }
  };
}

inline void Sampler::SetNoteVolume_Midi( uint8_t data1) {
  volume_midi[ selectedNote + 1 ] = data1;
#ifdef DEBUG_MIDI
  DEBF("Sampler - Note[%d].midi_vol: %d\n",  selectedNote, data1 );
#endif
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


inline void Sampler::NoteOn( uint8_t note, uint8_t vol ) {

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

inline void Sampler::NoteOff( uint8_t note ) {
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

void Sampler::SetPlaybackSpeed( float value ) {
  value = pow( 2.0f, 4.0f * (value - 0.5) );
  DEBF( "SetPlaybackSpeed: %0.2f\n", value );
  sampler_playback = value;
}

void Sampler::SetProgram( uint8_t prog ) {
  progNumber = prog ;
  Init();
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
   *left  = fclamp(signal_l * _volume, -1.0f, 1.0f);
   *right = fclamp(signal_r * _volume, -1.0f, 1.0f);
  // *left  = fast_shape(signal_l * _volume);
  // *right = fast_shape(signal_r * _volume);
}
