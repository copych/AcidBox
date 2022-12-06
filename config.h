#define PROG_NAME       "ESP32 AcidBox"
#define VERSION         "v0.2"

//#define DEBUG_ON
#define MIDI_ON
//#define MIDI_VIA_SERIAL // in style of Hairless MIDI, this will block Serial debugging

#define MAX_CUTOFF_FREQ 3000.0f
#define MIN_CUTOFF_FREQ 150.0f

//#define SH1106 //OLED display type: SH1106 or SSD1306, if you select none, no display code will be compiled 
//#define SSD1306   // uncomment if you have such kind
#define I2C_SDA         21
#define I2C_SCL         22

#define SAMPLE_RATE     44100
#define DIV_SAMPLE_RATE 2.267573e-5 // 1/44100
#define WAVE_SIZE       2048 // samples used for waveforms 
#define DMA_BUF_LEN     32
#define DMA_NUM_BUF     2

#define I2S_BCLK_PIN    5
#define I2S_WCLK_PIN    19
#define I2S_DOUT_PIN    18

#define MIDIRX_PIN      4
#define MIDITX_PIN      0

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

#define TWOPI PI*2.0f
#define MIDI_NORM 1.0f/127.0f



#define FORMAT_LITTLEFS_IF_FAILED true
#define CONFIG_LITTLEFS_CACHE_SIZE 512

/* use define to dump midi data */
//#define DEBUG_SAMPLER


#define PSRAM_BUFFER_SIZE 900000
// If Blocksize us set to 2048, the limit of SAMPLECNT is 12
#define PRELOADSIZE  (1024*2)  // always have this sample data preloaded to have no delay at the beginning of the note
#define BLOCKSIZE  (1024*2) /* only multiples of 2, otherwise the rest will not work */
#define SAMPLECNT 12

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))


// Debugging macros
#ifdef MIDI_VIA_SERIAL
  #ifndef DEB
    #ifdef DEBUG_ON
      #define DEB(...) oled.clearDisplay(); oled.setCursor(0, 0); oled.print(__VA_ARGS__); oled.display()
      #define DEBF(...) oled.clearDisplay(); oled.setCursor(0, 0); oled.printf(__VA_ARGS__); oled.display() 
      #define DEBUG(...) oled.clearDisplay(); oled.setCursor(0, 0); oled.println(__VA_ARGS__); oled.display()
    #else
      #define DEB(...)
      #define DEBF(...)
      #define DEBUG(...)
    #endif
  #endif
#else
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
#endif
