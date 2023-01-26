/*
* 
* AcidBox
* ESP32 headless acid combo of 2 x 303 + 1 x 808 like synths. MIDI driven. I2S output. No indication. Uses both cores of ESP32.
* 
* To build the thing
* You will need an ESP32 with PSRAM (ESP32 WROVER module). Preferrable an external DAC, like PCM5102. In ArduinoIDE Tools menu select:
* 
* * Board: ESP32 Dev Module
* * Partition scheme: No OTA (1MB APP/ 3MB SPIFFS)
* * PSRAM: enabled
*
* Also you will need to upload samples from /data folder to the ESP32 flash. To do so follow the instructions:
* https://github.com/lorol/LITTLEFS#arduino-esp32-littlefs-filesystem-upload-tool
* And then use Tools -> ESP32 Sketch Data Upload
*
*/

#include "config.h"

#include "driver/i2s.h"
#include "fx_delay.h"
#ifndef NO_PSRAM
  #include "fx_reverb.h"
#endif
#include "compressor.h"
#include "synthvoice.h"
#include "sampler.h"
#include <Wire.h>

#ifdef MIDI_ON
  #include <MIDI.h>
  #ifdef MIDI_VIA_SERIAL
    // default settings for Hairless midi is 115200 8-N-1
    struct CustomBaudRateSettings : public MIDI_NAMESPACE::DefaultSerialSettings {
      static const long BaudRate = 115200;
    };
    MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings> serialMIDI(Serial);
    MIDI_NAMESPACE::MidiInterface<MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings>> MIDI((MIDI_NAMESPACE::SerialMIDI<HardwareSerial, CustomBaudRateSettings>&)serialMIDI);
  #else
    // MIDI port on UART2,   pins 16 (RX) and 17 (TX) prohibited, as they are used for PSRAM
    
  struct Serial2MIDISettings : public midi::DefaultSettings{
    static const long BaudRate = 31250;
    static const int8_t RxPin  = MIDIRX_PIN;
    static const int8_t TxPin  = MIDITX_PIN; 
  };
  
  HardwareSerial MIDISerial(2);
  MIDI_CREATE_CUSTOM_INSTANCE( HardwareSerial, MIDISerial, MIDI, Serial2MIDISettings );
  
  #endif
#endif

const i2s_port_t i2s_num = I2S_NUM_0; // i2s port number
  
// lookuptables
static float midi_pitches[128];
static float midi_phase_steps[128];
static float midi_2048_steps[128];
static float exp_2048[WAVE_SIZE];
static float square_2048[WAVE_SIZE];
static float tanh_2048[WAVE_SIZE];
static uint32_t last_reset = 0;

// Audio buffers of all kinds
static float synth_buf[2][DMA_BUF_LEN]; // 2 * 303 mono
static float drums_buf_l[DMA_BUF_LEN];  // 808 stereo L
static float drums_buf_r[DMA_BUF_LEN];  // 808 stereo R
static float mix_buf_l[DMA_BUF_LEN];    // mix L channel
static float mix_buf_r[DMA_BUF_LEN];    // mix R channel
static union { // a dirty trick, instead of true converting
  int16_t _signed[DMA_BUF_LEN * 2];
  uint16_t _unsigned[DMA_BUF_LEN * 2];
} out_buf; // i2s L+R output buffer

volatile boolean processing = false;

#ifndef NO_PSRAM
static float rvb_k1, rvb_k2, rvb_k3;
#endif
static float dly_k1, dly_k2, dly_k3;

size_t bytes_written; // i2s
static uint32_t c1=0, c2=0, c3=0, d1=0, d2=0, d3=0, prescaler; // debug timing

// tasks for Core0 and Core1
TaskHandle_t SynthTask1;
TaskHandle_t SynthTask2;

// 303-like synths
SynthVoice Synth1(0); // use synth_buf[0]
SynthVoice Synth2(1); // use synth_buf[1]

// 808-like drums
Sampler Drums(DRUMKITCNT , DEFAULT_DRUMKIT); // first arg = total number of sample sets, second = starting drumset [0 .. total-1]

// Global effects
  FxDelay Delay;
#ifndef NO_PSRAM
  FxReverb Reverb;
#endif
Compressor Comp;

// Core0 task
static void audio_task1(void *userData) {
    while(1) {
        // this part of the code never intersects with mixer buffers
        // this part of the code is operating with shared resources, so we should make it safe
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)){
            c1=micros();
            Synth1.Generate(); 
            d1=micros()-c1;
            xTaskNotifyGive(SynthTask2); // if you have glitches, you may want to place this string in the end of audio_task1
        }
        taskYIELD();
    }
}

// task for Core1, which tipically runs user's code on ESP32
static void audio_task2(void *userData) {
    while(1) {
        // we can run it together with synth(), but not with mixer()
        c2 = micros();
        drums_generate();
 //     Synth1.Generate(); 
        Synth2.Generate();
        d2 = micros() - c2;
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) { // we need all the generators to fill the buffers here, so we wait
          c3 = micros();
          mixer(); // actually we could send Notify before mixer() is done, but then we'd need tic-tac buffers for generation. Todo maybe
          d3 = micros() - c3;
          xTaskNotifyGive(SynthTask1); 
        }        
        
        i2s_output();
        
        taskYIELD();
    }
}

void setup(void) {

  btStop();
  
#ifdef MIDI_ON
  #ifdef MIDI_VIA_SERIAL 
    Serial.begin(115200); 
  #else
    pinMode( MIDIRX_PIN , INPUT_PULLDOWN); 
    MIDISerial.begin( 31250, SERIAL_8N1, MIDIRX_PIN, MIDITX_PIN ); // midi port
  #endif
#else
#endif

#ifdef DEBUG_ON
#ifndef MIDI_VIA_SERIAL
  Serial.begin(115200);
#endif
#endif
  /*
  for (uint8_t i = 0; i < GPIO_BUTTONS; i++) {
    pinMode(buttonGPIOs[i], INPUT_PULLDOWN);
  }
  */
  
  buildTables();
  

#ifdef MIDI_ON
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandleControlChange(handleCC);
  MIDI.setHandleProgramChange(handleProgramChange);
  MIDI.begin(MIDI_CHANNEL_OMNI);
#endif

#ifndef NO_PSRAM
  Reverb.Init();  
#endif
  Delay.Init();
  Drums.Init();
  Synth1.Init();
  Synth2.Init();
  Comp.Init(SAMPLE_RATE);
#ifdef JUKEBOX
  init_midi();
#endif

 // silence while we haven't loaded anything reasonable
  for (int i=0; i < DMA_BUF_LEN; i++) { 
    drums_buf_l[i] = 0.0f ;
    drums_buf_r[i] = 0.0f ; 
    synth_buf[0][i] = 0.0f ; 
    synth_buf[1][i] = 0.0f ; 
    out_buf._signed[i*2] = 0 ;
    out_buf._signed[i*2+1] = 0 ;
    mix_buf_l[i] = 0.0f;
    mix_buf_r[i] = 0.0f;
  }
  
  i2sInit(); 
  i2s_write(i2s_num, out_buf._signed, sizeof(out_buf._signed), &bytes_written, portMAX_DELAY);
  
  xTaskCreatePinnedToCore( audio_task1, "SynthTask1", 8000, NULL, 1, &SynthTask1, 0 );
  xTaskCreatePinnedToCore( audio_task2, "SynthTask2", 8000, NULL, 1, &SynthTask2, 1 );
  
  // somehow we should allow tasks to run
  xTaskNotifyGive(SynthTask1);
  xTaskNotifyGive(SynthTask2);
  processing = true;
}

static uint32_t last_ms = micros();

void loop() { // default loopTask running on the Core1
  // you can still place some of your code here
  // or   vTaskDelete(NULL);
  
  // processButtons();
   
  #ifdef MIDI_ON
  MIDI.read();
  #endif
 
  #ifdef JUKEBOX
  if (micros()-last_ms>1000) {
    last_ms = micros();
    run_tick();
    myRandomAddEntropy((uint16_t)(last_ms & 0x0000FFFF));
  }
  #endif
  
  //DEBF ("%d %d %d \r\n" , d1, d2, d3);

  taskYIELD(); // breath for all the rest of the Core1 
}
