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
#include "fx_delay.h"
#ifndef NO_PSRAM
#include "fx_reverb.h"
#endif
#include "compressor.h"
#include "synthvoice.h"
#include "sampler.h"
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
static  float     param[POT_NUM];

// Audio buffers of all kinds
volatile int current_gen_buf = 0; // set of buffers for generation
volatile int current_out_buf = 1 - 0; // set of buffers for output
// static float DRAM_ATTR WORD_ALIGNED_ATTR  synth1_buf[2][DMA_BUF_LEN];    // synth1 mono
// static float DRAM_ATTR WORD_ALIGNED_ATTR  synth2_buf[2][DMA_BUF_LEN];    // synth2 mono
// static float DRAM_ATTR WORD_ALIGNED_ATTR  drums_buf_l[2][DMA_BUF_LEN];   // drums L
// static float DRAM_ATTR WORD_ALIGNED_ATTR  drums_buf_r[2][DMA_BUF_LEN];   // drums R
static float DRAM_ATTR WORD_ALIGNED_ATTR  mix_buf_l[2][DMA_BUF_LEN];     // mix L channel
static float DRAM_ATTR WORD_ALIGNED_ATTR  mix_buf_r[2][DMA_BUF_LEN];     // mix R channel
static union {                              // a dirty trick, instead of true converting
  int16_t WORD_ALIGNED_ATTR _signed[DMA_BUF_LEN * 2];
  uint16_t WORD_ALIGNED_ATTR _unsigned[DMA_BUF_LEN * 2];
} out_buf[2];                               // i2s L+R output buffer
size_t bytes_written;                       // i2s result

#ifndef NO_PSRAM
volatile float rvb_k1, rvb_k2, rvb_k3;
#endif
volatile float dly_k1, dly_k2, dly_k3;

// tasks for Core0 and Core1
TaskHandle_t SynthTask1;
TaskHandle_t SynthTask2;

// 303-like synths
SynthVoice Synth1(0); 
SynthVoice Synth2(1); 

// 808-like drums
Sampler Drums( DEFAULT_DRUMKIT ); // argument: starting drumset [0 .. total-1]

// Global effects
FxDelay Delay;
#ifndef NO_PSRAM
FxReverb Reverb;
#endif
Compressor Comp;

hw_timer_t * timer1 = NULL;            // Timer variables
hw_timer_t * timer2 = NULL;            // Timer variables
portMUX_TYPE timer1Mux = portMUX_INITIALIZER_UNLOCKED; 
portMUX_TYPE timer2Mux = portMUX_INITIALIZER_UNLOCKED; 
volatile boolean timer1_fired = false;
volatile boolean timer2_fired = false;

/*
 * Timer interrupt handler **********************************************************************************************************************************
*/

void IRAM_ATTR onTimer1() {
   portENTER_CRITICAL_ISR(&timer1Mux);
   timer1_fired = true;
   portEXIT_CRITICAL_ISR(&timer1Mux);
}
 
void IRAM_ATTR onTimer2() {
   portENTER_CRITICAL_ISR(&timer2Mux);
   timer2_fired = true;
   portEXIT_CRITICAL_ISR(&timer2Mux);
}

/* 
 * Core Tasks ************************************************************************************************************************
*/
// Core0 task 
// static void audio_task1(void *userData) {
static void IRAM_ATTR audio_task1(void *userData) {
  
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
      General::mixer(); 
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
// static void IRAM_ATTR audio_task2(void *userData) {
static void IRAM_ATTR audio_task2(void *userData) {
  while (true) {
    taskYIELD();
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
    
    if (timer2_fired) {
      timer2_fired = false;
#ifdef TEST_POTS      
       readPots();
#endif
       
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

  btStop(); // we don't want bluetooth to consume our precious cpu time 

  MidiInit(); // init midi input and handling of midi events

  /*
    for (int i = 0; i < GPIO_BUTTONS; i++) {
    pinMode(buttonGPIOs[i], INPUT_PULLDOWN);
    }
  */

  buildTables();

  for (int i = 0; i < POT_NUM; i++) pinMode( POT_PINS[i] , INPUT);

  Synth1.Init();
  Synth2.Init();
  Drums.Init();
#ifndef NO_PSRAM
  Reverb.Init();
#endif
  Delay.Init();
  Comp.Init(SAMPLE_RATE);
#ifdef JUKEBOX
  init_midi(); // AcidBanger function
#endif

  // silence while we haven't loaded anything reasonable
  for (int i = 0; i < DMA_BUF_LEN; i++) {
    // drums_buf_l[current_gen_buf][i] = 0.0f ;
    // drums_buf_r[current_gen_buf][i] = 0.0f ;
    // synth1_buf[current_gen_buf][i] = 0.0f ;
    // synth2_buf[current_gen_buf][i] = 0.0f ;
    out_buf[current_out_buf]._signed[i * 2] = 0 ;
    out_buf[current_out_buf]._signed[i * 2 + 1] = 0 ;
    mix_buf_l[current_out_buf][i] = 0.0f;
    mix_buf_r[current_out_buf][i] = 0.0f;
  }

  i2sInit();
  // i2s_write(i2s_num, out_buf[current_out_buf]._signed, sizeof(out_buf[current_out_buf]._signed), &bytes_written, portMAX_DELAY);

  //xTaskCreatePinnedToCore( audio_task1, "SynthTask1", 8000, NULL, (1 | portPRIVILEGE_BIT), &SynthTask1, 0 );
  //xTaskCreatePinnedToCore( audio_task2, "SynthTask2", 8000, NULL, (1 | portPRIVILEGE_BIT), &SynthTask2, 1 );
  xTaskCreatePinnedToCore( audio_task1, "SynthTask1", 5000, NULL, 1, &SynthTask1, 0 );
  xTaskCreatePinnedToCore( audio_task2, "SynthTask2", 5000, NULL, 1, &SynthTask2, 1 );

  // somehow we should allow tasks to run
  xTaskNotifyGive(SynthTask1);
  //  xTaskNotifyGive(SynthTask2);

#if ESP_ARDUINO_VERSION_MAJOR < 3 
  // timer interrupt
  /*
  timer1 = timerBegin(0, 80, true);               // Setup timer for midi
  timerAttachInterrupt(timer1, &onTimer1, true);  // Attach callback
  timerAlarmWrite(timer1, 4000, true);            // 4000us, autoreload
  timerAlarmEnable(timer1);
  */
  timer2 = timerBegin(1, 80, true);               // Setup general purpose timer
  timerAttachInterrupt(timer2, &onTimer2, true);  // Attach callback
  timerAlarmWrite(timer2, 200000, true);          // 200ms, autoreload 
  timerAlarmEnable(timer2);
  
#else 
  timer2 = timerBegin(1000000);               // Setup general purpose timer
  timerAttachInterrupt(timer2, &onTimer2);  // Attach callback
  timerAlarm(timer2, 200000, true, 0);          // 200ms, autoreload
#endif
DEBUG("setup done");
}

static uint32_t last_ms = micros();

/* 
 *  Finally, the LOOP () ***********************************************************************************************************
*/

void loop() { // default loopTask running on the Core1
  // you can still place some of your code here
  // or   vTaskDelete(NULL);
  
  // processButtons();
  regular_checks();    
  taskYIELD(); // this can wait
}

/* 
 *  Some debug and service routines *****************************************************************************************************************************
*/

void readPots() {
  static const float snap = 0.003f;
  static int i = 0;
  float tmp;
  static const float NORMALIZE_ADC = 1.0f / 4096.0f;
//read one pot per call
  tmp = (float)analogRead(POT_PINS[i]) * NORMALIZE_ADC;
  if (fabs(tmp - param[i]) > snap) {
    param[i] = tmp;
  //  paramChange(i, tmp);
  }

  i++;
  // if (i >= POT_NUM) i=0;
  i %= POT_NUM;
}

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
    default:
      {}
  }
}


#ifdef JUKEBOX
void jukebox_tick() {
  run_tick();
  myRandomAddEntropy((uint16_t)(micros() & 0x0000FFFF));
}
#endif


void regular_checks() {
  timer1_fired = false;
  
#ifdef MIDI_VIA_SERIAL
  MIDI.read();
#endif

#ifdef MIDI_VIA_SERIAL2
  MIDI2.read();
#endif
  
#ifdef JUKEBOX
  jukebox_tick();
#endif
}