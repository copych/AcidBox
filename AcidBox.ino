/*

  AcidBox
  ESP32 acid combo of 303 + 303 + 808 like synths. MIDI driven. I2S output to DAC. No indication. Uses both cores of ESP32.

  To build the thing
  You will need an ESP32 with PSRAM (ESP32 WROVER module). Preferrable an external DAC, like PCM5102. In ArduinoIDE Tools menu select:

* * Board: "ESP32 Dev Module" or "ESP32S3 Dev Module"
* * Partition scheme: No OTA (1MB APP/ 3MB SPIFFS)
* * PSRAM: "enabled" or "OPI PSRAM" or what type you have


  !!!!!!!! ATTENTION !!!!!!!!!
  You will need to upload samples from /data folder to the ESP32 flash, otherwise you'll only have 40kB samples from samples.h. 
  To upload samples follow the instructions:
  
  https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool
  And then use Tools -> ESP32 Sketch Data Upload

*/
#pragma GCC optimize ("O2")
#include "config.h"
#include "tables.h"
#include "general.h"
#include "src/fx/fx_delay.h"
#ifndef NO_PSRAM
#include "src/fx/fx_reverb.h"
#endif
#include "src/fx/compressor.h"
#include "src/synth/synthvoice.h"
#include "src/sampler/sampler.h"
#include "src/mixer/mixer.h"
#include "src/sequencer/looper.h"
#include "src/controls/controls.h"
#include "src/gui/gui.h"
#include <Wire.h>
#ifdef DEBUG_TIMING
#include "debug_timing.h"
#endif


// =============================================================== MIDI interfaces ===============================================================

#if defined MIDI_VIA_SERIAL2 || defined MIDI_VIA_SERIAL
#include <MIDI.h>
#endif

#ifdef MIDI_VIA_SERIAL

  struct CustomBaudRateSettings : public MIDI_NAMESPACE::DefaultSettings {
    static const long BaudRate = 115200;
    static const bool Use1ByteParsing = false;
  };

  MIDI_NAMESPACE::SerialMIDI<MIDI_PORT_TYPE, CustomBaudRateSettings> serialMIDI(MIDI_PORT);
  MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<MIDI_PORT_TYPE, CustomBaudRateSettings>> MIDI((MIDI_NAMESPACE::SerialMIDI<MIDI_PORT_TYPE, CustomBaudRateSettings>&)serialMIDI);

#endif

#ifdef MIDI_VIA_SERIAL2
// MIDI port on UART2,   pins 16 (RX) and 17 (TX) prohibited on ESP32, as they are used for PSRAM
struct Serial2MIDISettings : public midi::DefaultSettings {
  static const long BaudRate = 31250;
  static const int8_t RxPin  = MIDIRX_PIN;
  static const int8_t TxPin  = MIDITX_PIN;
  static const bool Use1ByteParsing = false;
};
MIDI_NAMESPACE::SerialMIDI<HardwareSerial> Serial2MIDI2(Serial2);
MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, Serial2MIDISettings>> MIDI2((MIDI_NAMESPACE::SerialMIDI<HardwareSerial, Serial2MIDISettings>&)Serial2MIDI2);
#endif

// service variables and arrays
static  uint32_t  last_reset = 0;

// Audio buffers of all kinds
volatile int current_gen_buf = 0; // set of buffers for generation
volatile int current_out_buf = 1 - 0; // set of buffers for output

static union {                              // a dirty trick, instead of true converting
  int16_t WORD_ALIGNED_ATTR _signed[DMA_BUF_LEN * 2];
  uint16_t WORD_ALIGNED_ATTR _unsigned[DMA_BUF_LEN * 2];
} out_buf[2];                               // i2s L+R output buffer
size_t bytes_written;                       // i2s result

// tasks for Core0 and Core1
TaskHandle_t SynthTask1;
TaskHandle_t SynthTask2;

// 303-like synths
SynthVoice Synth1(0, &current_gen_buf); 
SynthVoice Synth2(1, &current_gen_buf); 

// 808-like drums
Sampler Drums(DEFAULT_DRUMKIT, &current_gen_buf); // argument: starting drumset [0 .. total-1]

// Global effects
FxDelay Delay;
Compressor Comp;
#ifndef NO_PSRAM
FxReverb Reverb;
#endif

// Mixer
#ifndef NO_PSRAM
Mixer mixer(&Synth1, &Synth2, &Drums, &Delay, &Comp, &Reverb, &current_out_buf);
#else
Mixer mixer(&Synth1, &Synth2, &Drums, &Delay, &Comp, &current_out_buf);
#endif

hw_timer_t * timer1 = NULL;            // Timer variables
portMUX_TYPE timer1Mux = portMUX_INITIALIZER_UNLOCKED; 
volatile boolean timer1_fired = false;

using namespace performer;
Looper Performer;

OledGUI gui;

/*
 * Timer interrupt handler **********************************************************************************************************************************
*/

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timer1Mux);
   timer1_fired = true;
   portEXIT_CRITICAL_ISR(&timer1Mux);
}

/*
 * Muxed controls ====================================================================================================================
*/

UIControls controls;

/* 
/*
 * Core Tasks ************************************************************************************************************************
*/
// Core0 task 
// static void audio_task(void *userData) {
static void IRAM_ATTR audio_task(void *userData) {
  DEBUG ("core 0 audio task run");
  vTaskDelay(20);
  
  while (true) {
    taskYIELD(); 
//    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) { // we need all the generators to fill the buffers here, so we wait

#ifdef DEBUG_TIMING
      Debug::c0t = micros();
#endif
      
//      taskYIELD(); 
      
      current_gen_buf = current_out_buf;      // swap buffers
      current_out_buf = 1 - current_gen_buf;
      
    //  xTaskNotifyGive(SynthTask2);            // if we are here, then we've already received a notification from task2
      
#ifdef DEBUG_TIMING
      RECORD_TIME(Debug::s1t, Synth1.generate(), Debug::s1T)  
#else
      Synth1.generate();
#endif

#ifdef DEBUG_TIMING
      RECORD_TIME(Debug::s2t, Synth2.generate(), Debug::s2T)  
#else
      Synth2.generate();
#endif

  //    taskYIELD(); 

#ifdef DEBUG_TIMING
      RECORD_TIME(Debug::drt, Drums.generate(), Debug::drT)  
#else
      Drums.generate();
#endif



#ifdef DEBUG_TIMING      
      Debug::c1t = micros();
      Debug::fxt = micros();
#endif      
      mixer.mix(); 
#ifdef DEBUG_TIMING      
      Debug::fxT = micros() - Debug::fxt;
#endif            
      i2s_output();

 //   }
    
   // taskYIELD();

    taskYIELD();
#ifdef DEBUG_TIMING 
    Debug::c0T = micros() - Debug::c0t;
#endif   
  }
}

// task for Core1, which tipically runs user's code on ESP32
// static void IRAM_ATTR control_task(void *userData) {
static void IRAM_ATTR control_task(void *userData) {
  DEBUG ("core 1 control task run");
  vTaskDelay(20);
  while (true) {
    taskYIELD();
    
    regular_checks();    
 /*   
    if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) { // wait for the notification from the SynthTask1
      taskYIELD();
      xTaskNotifyGive(SynthTask1); 
    }    
 */
#ifdef DEBUG_TIMING
    Debug::c1T = micros() - Debug::c1t;
    Debug::art = micros();
#endif

    if (timer1_fired) {
      timer1_fired = false;
       
#ifdef DEBUG_TIMING
      DEBF ("CORE micros: synt1, synt2, drums, mixer, DMA_LEN\t%d\t%d\t%d\t%d\t%d\r\n" , Debug::s1T, Debug::s2T, Debug::drT, Debug::fxT, DMA_BUF_TIME);
        //    DEBF ("TaskCore0=%dus TaskCore1=%dus DMA_BUF=%dus\r\n" , Debug::c0T , Debug::c1T , DMA_BUF_TIME);
        //    DEBF ("AllTheRestCore1=%dus\r\n" , Debug::arT);
#endif
    }    
    
//    taskYIELD();

#ifdef DEBUG_TIMING
    Debug::arT = micros() - Debug::art;
#endif
  }
}


/* 
 *  Quite an ordinary SETUP() *******************************************************************************************************************************
*/

void setup(void) {

#ifdef DEBUG_ON 
  DEBUG_PORT.begin(115200); 
#endif
  delay(200);
  randomSeed(esp_random());   // it depends on bluetooth
  btStop();                   // now we can turn bluetooth off
  
  MidiInit(); // init midi input and handling of midi events
 

  /*
    for (int i = 0; i < GPIO_BUTTONS; i++) {
    pinMode(buttonGPIOs[i], INPUT_PULLDOWN);
    }
  */

  buildTables();

  Synth1.Init();
  Synth2.Init();
  Drums.Init();
#ifndef NO_PSRAM
  Reverb.Init();
#endif
  Delay.Init();
  Comp.Init(SAMPLE_RATE);
#ifdef JUKEBOX
  //init_midi(); // AcidBanger function
#endif

  // silence while we haven't loaded anything reasonable
  for (int i = 0; i < DMA_BUF_LEN; i++) {
    out_buf[current_out_buf]._signed[i * 2] = 0;
    out_buf[current_out_buf]._signed[i * 2 + 1] = 0;
  }


  // test environment for performance measurements
  testSetup();
  
  // setup encoders and buttons
  controls.begin();

  // start display
  gui.begin();

  // start audio output
  i2sInit();
  // i2s_write(i2s_num, out_buf[current_out_buf]._signed, sizeof(out_buf[current_out_buf]._signed), &bytes_written, portMAX_DELAY);

  //xTaskCreatePinnedToCore( audio_task, "SynthTask1", 8000, NULL, (1 | portPRIVILEGE_BIT), &SynthTask1, 0 );
  //xTaskCreatePinnedToCore( control_task, "SynthTask2", 8000, NULL, (1 | portPRIVILEGE_BIT), &SynthTask2, 1 );
  xTaskCreatePinnedToCore( audio_task, "SynthTask1", 5000, NULL, 2, &SynthTask1, 0 );
  xTaskCreatePinnedToCore( control_task, "SynthTask2", 6000, NULL, 1, &SynthTask2, 1 );

  // if we use semaphores and so on
  //  xTaskNotifyGive(SynthTask1);
  //  xTaskNotifyGive(SynthTask2);

#if ESP_ARDUINO_VERSION_MAJOR < 3 
  // timer interrupt
  timer1 = timerBegin(1, 80, true);               // Setup general purpose timer
  timerAttachInterrupt(timer1, &onTimer1, true);  // Attach callback
  timerAlarmWrite(timer1, 200000, true);          // 200ms, autoreload 
  timerAlarmEnable(timer1);  
#else 
  timer1 = timerBegin(1000000);               // Setup general purpose timer at 1MHz
  timerAttachInterrupt(timer1, &onTimer1);    // Attach callback
  timerAlarm(timer1, 200000, true, 0);        // 200ms, autoreload
#endif

#ifdef JUKEBOX_PLAY_ON_START
  Performer.play();	            // Start playing from the 0 position
#endif

  DEBUG("setup done");
}

/* 
 *  Finally, the LOOP () ***********************************************************************************************************
*/

void loop() { // default loopTask running on the Core1
  // you can still place some of your code here
  // or   vTaskDelete(NULL);
  
  controls.polling();
  
  taskYIELD(); // this can wait
}

/* 
 *  Some debug and service routines *****************************************************************************************************************************
*/


void paramChange(uint8_t paramNum, float paramVal) {
  // paramVal === param[paramNum];
  DEBF ("param %d val %0.4f\r\n" , paramNum, paramVal);
  paramVal *= 127.0;
  switch (paramNum) {
    case 0:
      //set_bpm( 40.0f + (paramVal * 160.0f));
      Synth2.ParseCC(CC_303_CUTOFF, paramVal);
      break;
    case 1:
      Synth2.ParseCC(CC_303_RESO, paramVal);
      break;
    case 2:
      Synth2.ParseCC(CC_303_OVERDRIVE, paramVal);
      Synth2.ParseCC(CC_303_DISTORTION, paramVal);
      break;
    case 3:
      Synth2.ParseCC(CC_303_ENVMOD_LVL, paramVal);
      break;
    case 4:
      Synth2.ParseCC(CC_303_ACCENT_LVL, paramVal);
      break;
    case 8:
      //set_bpm( 40.0f + (paramVal * 160.0f));
      Synth2.ParseCC(CC_303_CUTOFF, paramVal);
      break;
    case 9:
      Synth2.ParseCC(CC_303_RESO, paramVal);
      break;
    case 10:
      Synth2.ParseCC(CC_303_OVERDRIVE, paramVal);
      Synth2.ParseCC(CC_303_DISTORTION, paramVal);
      break;


    default:
      {}
  }
}



void regular_checks() {  
#ifdef MIDI_VIA_SERIAL
  MIDI.read();
#endif

#ifdef MIDI_VIA_SERIAL2
  MIDI2.read();
#endif
  
#ifdef JUKEBOX
  Performer.looperTask();
#endif

  gui.draw();
}

void testSetup() {
  
  Performer.setPpqn(96);         // pulses per quarter note
  Performer.setSwing(0.0);       // -1.0 .. 1.0 moving odd 16th notes forward and back. 0.667 is duplets; 0.0 means straight 16th
  Performer.setBpm(130);         // BPM
  Performer.setLoopSteps(16);    // Loop length, 16th notes
  
  DEBF("SEQ: add track: %d \r\n", Performer.addTrack(TRACK_MONO, 1));
  DEBF("SEQ: add track: %d \r\n", Performer.addTrack(TRACK_MONO, 2));
  DEBF("SEQ: add track: %d \r\n", Performer.addTrack(TRACK_DRUMS, 10));
  
  DEBF("SEQ: Track 0: add pattern: %d \r\n", Performer.Tracks[0].addPattern());
  DEBF("SEQ: Track 1: add pattern: %d \r\n", Performer.Tracks[1].addPattern());
  DEBF("SEQ: Track 2: add pattern: %d \r\n", Performer.Tracks[2].addPattern());
  
  Performer.Tracks[0].Patterns[0].generateNoteSet(0.5, 0.5);
  Performer.Tracks[0].Patterns[0].generateMelody(48, SLIDE_TEST_LOAD, 1.0 , 0.0, 1.0);
  
  Performer.Tracks[1].Patterns[0].generateNoteSet(0.5, 0.5);
  Performer.Tracks[1].Patterns[0].generateMelody(36, STYLE_TEST_LOAD, 1.0 , 0.0, 1.0);
  
  Performer.Tracks[2].Patterns[0].generateDrums( STYLE_TEST_LOAD, 1.0 , 0.0);

  Synth1.ParseCC(CC_303_WAVEFORM, 0);
  Synth1.ParseCC(CC_303_VOLUME, 60);
  Synth1.ParseCC(CC_303_PAN, 5);
  Synth1.ParseCC(CC_303_CUTOFF, 15);
  Synth1.ParseCC(CC_303_RESO, 100);
  Synth1.ParseCC(CC_303_OVERDRIVE, 50);
  Synth1.ParseCC(CC_303_DISTORTION, 0);
  Synth1.ParseCC(CC_303_ENVMOD_LVL, 120);
  Synth1.ParseCC(CC_303_ACCENT_LVL, 100);
  Synth1.ParseCC(CC_303_DELAY_SEND, 100);
  Synth1.ParseCC(CC_303_REVERB_SEND, 30);

  Synth2.ParseCC(CC_303_WAVEFORM, 127);
  Synth2.ParseCC(CC_303_VOLUME, 60);
  Synth2.ParseCC(CC_303_PAN, 122);
  Synth2.ParseCC(CC_303_CUTOFF, 20);
  Synth2.ParseCC(CC_303_RESO, 100);
  Synth2.ParseCC(CC_303_OVERDRIVE, 50);
  Synth2.ParseCC(CC_303_DISTORTION, 0);
  Synth2.ParseCC(CC_303_ENVMOD_LVL, 120);
  Synth2.ParseCC(CC_303_ACCENT_LVL, 50);
  Synth2.ParseCC(CC_303_DELAY_SEND, 100);
  Synth2.ParseCC(CC_303_REVERB_SEND, 60);
  
  DEBUG( Performer.Tracks[2].Patterns[0].toText());
  
}