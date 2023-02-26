#define PROG_NAME       "ESP32 AcidBox"
#define VERSION         "v.1.0.3"

//#define DEBUG_ON              // note that debugging eats ticks initially belonging to real-time tasks, so sound output will be spoild in most cases, turn it off for production build
//#define DEBUG_MASTER_OUT      // serial monitor plotter will draw the output waveform
//#define DEBUG_SAMPLER
//#define DEBUG_SYNTH
//#define DEBUG_JUKEBOX
//#define DEBUG_FX
//#define DEBUG_TIMING

//#define USE_INTERNAL_DAC      // use this for testing, SOUND QUALITY SACRIFICED: NOISY 8BIT STEREO
//#define NO_PSRAM              // if you don't have PSRAM on your board, then use this define, but REVERB TO BE SACRIFICED, SMALL DRUM KIT SAMPLES USED 

//#define MIDI_VIA_SERIAL         // use this option to enable Hairless MIDI on Serial port @115200 baud (USB connector), THIS WILL BLOCK SERIAL DEBUGGING as well
#define MIDI_VIA_SERIAL2        // use this option if you want to operate by MIDI @31250baud, UART2 (Serial2), 
#define MIDIRX_PIN      4       // this pin is used for input when MIDI_VIA_SERIAL2 defined (note that default pin 17 won't work with PSRAM)
#define MIDITX_PIN      0       // this pin will be used for output (not implemented yet) when MIDI_VIA_SERIAL2 defined


#define JUKEBOX                 // real-time endless auto-compose acid tunes
#define JUKEBOX_PLAY_ON_START   // should it play on power on, or should it wait for "boot" button to be pressed

float bpm = 130.0;

#define MAX_CUTOFF_FREQ 4000.0f
#define MIN_CUTOFF_FREQ 250.0f

#ifdef USE_INTERNAL_DAC
#define SAMPLE_RATE     22050   // price for increasing this value having NO_PSRAM is less delay time, you won't hear the difference at 8bit/sample
#else
#define SAMPLE_RATE     44100   // 44100 seems to be the right value, 48000 is also OK. Other values are not tested.
#endif

const float DIV_SAMPLE_RATE = 1.0f / (float)SAMPLE_RATE;
const float DIV_2SAMPLE_RATE = 0.5f / (float)SAMPLE_RATE;

#define WAVE_SIZE       1024        // samples used for lookup tables (it works pretty well down to 32 samples due to linear approximation, so listen and free some memory at your choice)
const float DIV_WAVE_SIZE = 1.0f / (float)WAVE_SIZE;
#define TANH_LOOKUP_MAX 5.0f        // maximum X argument value for tanh(X) lookup table, tanh(X)~=1 if X>4 
const float TANH_LOOKUP_COEF = (float)WAVE_SIZE / TANH_LOOKUP_MAX;
#define DMA_BUF_LEN     32          // there should be no problems with low values, down to 32 samples, 64 seems to be OK with some extra
#define DMA_NUM_BUF     2           // I see no reasom to set more than 2 DMA buffers, but...

const uint32_t DMA_BUF_TIME = (uint32_t)(1000000.0f / (float)SAMPLE_RATE * (float)DMA_BUF_LEN); // microseconds per buffer

#define I2S_BCLK_PIN    5
#define I2S_WCLK_PIN    19
#define I2S_DOUT_PIN    18

#define POT_NUM 3
const uint8_t POT_PINS[POT_NUM] = {34, 35, 36};

#define SYNTH1_MIDI_CHAN        1
#define SYNTH2_MIDI_CHAN        2

#define DRUM_MIDI_CHAN          10


/*
#define MUXED_BUTTONS   0
#define GPIO_BUTTONS    1
#define TOTAL_BUTTONS MUXED_BUTTONS+GPIO_BUTTONS 
#define LOGICAL_ON      HIGH

enum e_pins_t     { fn1 = MUXED_BUTTONS,  fn2,  fn3,  fn4, enc_but }; // mnemonics for bits, starting with the first after the muxed buttons
uint8_t buttonGPIOs[GPIO_BUTTONS]{ 23 }; // GPIO buttons
//e_pins_t gpio_pin;
bool autoFireEnabled = false;                 // should the buttons generate continious clicks when pressed longer than a longPressThreshold
bool lateClickEnabled = false;                // enable registering click after a longPress call
const unsigned long longPressThreshold = 800; // the threshold (in milliseconds) before a long press is detected
const unsigned long autoFireDelay = 500;      // the threshold (in milliseconds) between clicks if autofire is enabled
const unsigned long riseThreshold = 20;       // the threshold (in milliseconds) for a button press to be confirmed (i.e. debounce, not "noise")
const unsigned long fallThreshold = 10;       // debounce, not "noise", also this is the time, while new "touches" won't be registered
*/

const float TWOPI = PI*2.0f;
const float MIDI_NORM = 1.0f/127.0f;
const float DIV_TWOPI = 1.0f/TWOPI;
#define FORMAT_LITTLEFS_IF_FAILED true
//#define CONFIG_LITTLEFS_CACHE_SIZE 1024


#define DRUMKITCNT 6                // how many drumkits do we have
#ifdef NO_PSRAM
  #define RAM_SAMPLER_CACHE  40000    // compact sample set is 132kB, first 8 is ~38kB
  #define SAMPLECNT 8                 // how many sounds from the folder will be used
  #define DEFAULT_DRUMKIT 4           // /data/4/ folder
#else
  #define PSRAM_SAMPLER_CACHE 900000  // this cache's size must correspond to the largest sample set's size
  #define SAMPLECNT 12                // how many sounds from the folder will be used
  #define DEFAULT_DRUMKIT 0           // 0 has massive bassdrum
  // folder number in /data/ 
#endif

#ifndef LED_BUILTIN
#define LED_BUILTIN 0
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

// Debugging macros
#ifndef MIDI_VIA_SERIAL
  #ifndef DEB
    #ifdef DEBUG_ON
      #define DEB(...) Serial.print(__VA_ARGS__) 
      #define DEBF(...) Serial.printf(__VA_ARGS__) 
      #define DEBUG(...) Serial.println(__VA_ARGS__) 
    #else
      #define DEB(...)
      #define DEBF(...)
      #define DEBUG(...)
    #endif
  #endif
#else
      #define DEB(...)
      #define DEBF(...)
      #define DEBUG(...)
#endif
