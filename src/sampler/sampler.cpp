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
#include "general.h"

/* You only need to format LittleFS the first time you run a
   test or else use the LittleFS plugin to create a partition
   https://github.com/lorol/arduino-esp32LittleFS-plugin */

Sampler::Sampler(uint8_t progNow, volatile int *current_gen_buf) {
  program_tmp = progNow; progNumber = progNow;
  _current_gen_buf = current_gen_buf;
}


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
  repeat = acidbox_min(sampleInfoCount , repeat); // 12 (an octave) or less

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
      wav.dataSize = acidbox_min(wav.dataSize, buffPointer-oldPointer); // some samples have wrong header info
      
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

void IRAM_ATTR Sampler::generate() {
    for (int i=0; i < DMA_BUF_LEN; i++){
      Process( &drums_buf_l[*_current_gen_buf][i], &drums_buf_r[*_current_gen_buf][i] );      
    } 
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