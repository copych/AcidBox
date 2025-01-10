#pragma once
#ifndef CONFIG_H
#define CONFIG_H
#include "Arduino.h"

#define PROG_NAME       "ESP32 AcidBox"
#define VERSION         "v.1.4.0 S3"

#define BOARD_HAS_UART_CHIP

#define JUKEBOX                 // real-time endless auto-compose acid tunes
#define JUKEBOX_PLAY_ON_START   // should it play on power on, or should it wait for "boot" button to be pressed
#define MIDI_RAMPS              // this is what makes automated Cutoff-Reso-FX turn
//#define TEST_POTS               // experimental interactivity with potentiometers connected to POT_PINS[] defined below

//#define USE_INTERNAL_DAC      // use this for testing, SOUND QUALITY SACRIFICED: NOISY 8BIT STEREO
//#define NO_PSRAM              // if you don't have PSRAM on your board, then use this define, but REVERB TO BE SACRIFICED, ONE SMALL DRUM KIT SAMPLES USED 

//#define FLASH_LED               // flash built-in LED
//#define LOLIN_RGB               // Flashes the LOLIN S3 built-in RGB-LED

#define DEBUG_ON              // note that debugging eats ticks initially belonging to real-time tasks, so sound output will be spoild in most cases, turn it off for production build
//#define DEBUG_MASTER_OUT      // serial monitor plotter will draw the output waveform
//#define DEBUG_SAMPLER
//#define DEBUG_SYNTH
//#define DEBUG_JUKEBOX
//#define DEBUG_FX
#define DEBUG_TIMING
//#define DEBUG_MIDI

//#define MIDI_VIA_SERIAL       // use this option to enable Hairless MIDI on Serial port @115200 baud (USB connector), THIS WILL BLOCK SERIAL DEBUGGING as well
//#define MIDI_VIA_SERIAL2        // use this option if you want to operate by standard MIDI @31250baud, UART2 (Serial2), 
#define MIDIRX_PIN      4       // this pin is used for input when MIDI_VIA_SERIAL2 defined (note that default pin 17 won't work with PSRAM)
#define MIDITX_PIN      15      // this pin will be used for output (not implemented yet) when MIDI_VIA_SERIAL2 defined

#define POT_NUM 3
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define I2S_BCLK_PIN    5       // I2S BIT CLOCK pin (BCL BCK CLK)
#define I2S_DOUT_PIN    6       // to I2S DATA IN pin (DIN D DAT)
#define I2S_WCLK_PIN    7       // I2S WORD CLOCK pin (WCK WCL LCK)
const uint8_t POT_PINS[POT_NUM] = {15, 16, 17};
#elif defined(CONFIG_IDF_TARGET_ESP32)
#define I2S_BCLK_PIN    5       // I2S BIT CLOCK pin (BCL BCK CLK)
#define I2S_WCLK_PIN    19      // I2S WORD CLOCK pin (WCK WCL LCK)
#define I2S_DOUT_PIN    18      // to I2S DATA IN pin (DIN D DAT)
const uint8_t POT_PINS[POT_NUM] = {34, 35, 36};
#endif


static float bpm = 130.0f;

#ifdef USE_INTERNAL_DAC
#define SAMPLE_RATE     22050   // price for increasing this value having NO_PSRAM is less delay time, you won't hear the difference at 8bit/sample
#else
#define SAMPLE_RATE     44100   // 44100 seems to be the right value, 48000 is also OK. Other values haven't been tested.
#endif

const float DIV_SAMPLE_RATE = 1.0f / (float)SAMPLE_RATE;
const float DIV_2SAMPLE_RATE = 0.5f / (float)SAMPLE_RATE;
const float TWO_DIV_16383 = 1.22077763e-04f;
const float MS_TO_S = 0.001f;

#define TABLE_BIT  		        10UL				// bits per index of lookup tables for waveforms, exp(), sin(), cos() etc. 10 bit means 2^10 = 1024 samples
#define TABLE_SIZE            (1<<TABLE_BIT)        // samples used for lookup tables (it works pretty well down to 32 samples due to linear approximation, so listen and free some memory at your choice)
#define TABLE_MASK  	        (TABLE_SIZE-1)        // strip MSB's and remain within our desired range of TABLE_SIZE
#define CYCLE_INDEX(i)        (((int32_t)(i)) & TABLE_MASK ) // this way we can operate with periodic functions or waveforms without phase-reset ("if's" are pretty costly in the matter of time)

const float DIV_TABLE_SIZE =  1.0f / (float)TABLE_SIZE;
const int HALF_TABLE =  TABLE_SIZE/2;


// illinear shaper, choose preferred parameters basing on your audial experience
#define SHAPER_USE_TANH             // use tanh() function to introduce illeniarity into the filter and compressor, it won't impact performance as this will be pre-calculated 
//#define SHAPER_USE_CUBIC              // use the cubic curve to introduce illeniarity into the filter and compressor, it won't impact performance as this will be pre-calculated

// curve will be pre-calculated within -X..X range, outside this interval the function is assumed to be flat
#define SHAPER_LOOKUP_MAX 5.0f        // maximum X argument value for tanh(X) lookup table, tanh(X)~=1 if X>4 
const float SHAPER_LOOKUP_COEF = (float)TABLE_SIZE / SHAPER_LOOKUP_MAX;
#define DMA_BUF_LEN     32          // there should be no problems with low values, down to 32 samples, 64 seems to be OK with some extra
#define DMA_NUM_BUF     2           // I see no reasom to set more than 2 DMA buffers, but...

const uint32_t DMA_BUF_TIME = (uint32_t)(1000000.0f / (float)SAMPLE_RATE * (float)DMA_BUF_LEN); // microseconds per buffer, used for debugging output of time-slots

#define SYNTH1_MIDI_CHAN        1
#define SYNTH2_MIDI_CHAN        2

#define DRUM_MIDI_CHAN          10

const float TWOPI = PI*2.0f;
const float MIDI_NORM = 1.0f/127.0f;
const float ONE_DIV_PI = 1.0f/PI;
const float ONE_DIV_TWOPI = 1.0f/TWOPI;

const float  PI_DIV_TWO     =       HALF_PI;
const float  NORM_RADIANS = ONE_DIV_TWOPI * TABLE_SIZE;

#define FORMAT_LITTLEFS_IF_FAILED true

#define GROUP_HATS  // if so, instruments CH_NUMBER and OH_NUMBER will terminate each other (sampler module)
#define CH_NUMBER  6 // closed hat instrument number in kit (for groupping, zero-based)
#define OH_NUMBER  7 // open hat instrument number in kit (for groupping, zero-based)

#ifdef NO_PSRAM
  #define RAM_SAMPLER_CACHE  40000    // bytes, compact sample set is 132kB, first 8 samples is ~38kB
  #define DEFAULT_DRUMKIT 4           // /data/4/ folder
  #define SAMPLECNT       8           // how many samples we prepare (here just 8)
#else
//  #define PRELOAD_ALL                 // allows operating all the samples in realtime
  #define PSRAM_SAMPLER_CACHE 3145728 // bytes, we are going to preload ALL the samples from FLASH to PSRAM
                                      // we divide samples by octaves to use modifiers to particular instruments, not just note numbers
                                      // i.e. we know that all the "C" notes in all octaves are bass drums, and CC_808_BD_TONE affects all BD's
  #define SAMPLECNT       (7 * 12)    // how many samples we prepare (8 octaves by 12 samples)
  #define DEFAULT_DRUMKIT 0           // in my /data /0 has a massive bassdrum , /6 = 808 samples
#endif

#define TINY 1e-32;

#ifndef LED_BUILTIN
#define LED_BUILTIN 0
#endif

#define ARRAY_SIZE(a) (sizeof(a)/sizeof(a[0]))

#if (defined ARDUINO_LOLIN_S3_PRO)
#undef BOARD_HAS_UART_CHIP
#endif

#if (defined BOARD_HAS_UART_CHIP)
  #define MIDI_PORT_TYPE HardwareSerial
  #define MIDI_PORT Serial
  #define DEBUG_PORT Serial
#else
  #if (ESP_ARDUINO_VERSION_MAJOR < 3)
    #define MIDI_PORT_TYPE HWCDC
    #define MIDI_PORT USBSerial
    #define DEBUG_PORT USBSerial
  #else
    #define MIDI_PORT_TYPE HardwareSerial
    #define MIDI_PORT Serial
    #define DEBUG_PORT Serial
  #endif
#endif

#ifdef MIDI_VIA_SERIAL
  #undef DEBUG_ON
#endif

// debug macros
#ifdef DEBUG_ON
  #define DEB(...)    DEBUG_PORT.print(__VA_ARGS__) 
  #define DEBF(...)   DEBUG_PORT.printf(__VA_ARGS__)
  #define DEBUG(...)  DEBUG_PORT.println(__VA_ARGS__)
#else
  #define DEB(...)
  #define DEBF(...)
  #define DEBUG(...)
#endif

// normalizing matrices for TB filter and distortion/overdrive pairs
#define NORM1_DEPTH 1.0f 
#define NORM2_DEPTH 1.0f

static const float wfolder_overdrive[16][16] = { // D-weighting curve linear amplitude
{4.321596, 6.677420, 9.351027, 12.337818, 15.274008, 17.178272, 20.258532, 22.640339, 23.268341, 25.133560, 25.689850, 27.329815, 26.931023, 27.971588, 28.773928, 27.811522},
{6.072484, 10.221110, 14.169627, 17.745028, 20.698469, 24.349220, 25.056787, 26.135130, 27.644402, 29.212200, 28.163837, 28.859060, 30.591475, 30.274736, 30.926729, 32.161110},
{8.636417, 13.038910, 17.829748, 23.371164, 25.444685, 26.327213, 28.255512, 29.594391, 29.098421, 29.936840, 31.134794, 32.169270, 32.045223, 32.963749, 32.976822, 32.455048},
{10.054599, 16.592342, 22.589405, 25.882214, 28.521395, 29.180340, 29.164886, 30.660505, 31.349100, 32.824554, 32.293663, 33.904400, 33.067791, 32.399799, 33.414825, 33.234741},
{15.011400, 23.227974, 31.648169, 35.058456, 32.518459, 30.586367, 30.851610, 33.365906, 34.632706, 35.548817, 35.342865, 33.191872, 33.392647, 35.636063, 37.346981, 36.876877},
{23.186680, 38.978333, 48.210861, 40.115742, 31.781157, 24.790371, 31.183672, 40.898441, 47.043743, 41.951683, 33.172577, 28.403805, 32.496960, 41.487301, 46.415443, 41.383484},
{29.932003, 47.438541, 56.828445, 41.501617, 26.867907, 20.738359, 32.649918, 49.848667, 54.199947, 42.713993, 27.205708, 22.805727, 34.331593, 49.077122, 54.067410, 40.862373},
{33.698540, 54.806038, 60.328083, 41.728024, 22.957489, 17.200394, 33.867798, 54.327579, 60.292271, 40.527672, 22.979492, 19.049171, 35.523216, 54.576229, 58.380924, 40.722004},
{35.641998, 57.294373, 64.910187, 41.493423, 20.014410, 14.921355, 34.554081, 58.559727, 62.141014, 41.429733, 19.964781, 16.598537, 35.357327, 58.005657, 62.420906, 40.056889},
{37.487209, 59.607437, 65.128052, 41.079487, 18.249022, 13.841405, 35.108574, 61.368694, 64.131561, 39.774368, 18.559950, 14.785675, 36.562847, 60.325825, 64.121376, 39.700741},
{37.041679, 60.903545, 66.883095, 41.005798, 17.416222, 13.260203, 36.083569, 60.961777, 64.818130, 40.642086, 17.360371, 14.153852, 36.164448, 62.356262, 64.683060, 39.244789},
{38.588360, 62.251549, 66.640533, 40.079689, 16.864496, 13.063670, 35.713974, 63.281395, 65.684242, 40.100368, 16.357441, 13.569439, 36.963696, 62.407402, 66.269562, 38.965614},
{38.136345, 62.866852, 66.513802, 40.857845, 16.487530, 12.740461, 35.939171, 62.047729, 66.705620, 39.801647, 16.169365, 13.296426, 37.040180, 63.597023, 64.751793, 38.943562},
{38.979004, 63.323586, 66.772980, 40.269608, 16.517096, 12.285855, 35.989716, 64.168343, 67.227440, 39.479458, 15.763931, 13.077268, 36.944458, 63.458538, 66.844772, 39.394936},
{38.541519, 62.366325, 66.909378, 40.739624, 16.021036, 12.396644, 36.157871, 63.634758, 66.528000, 39.438278, 15.723740, 12.900184, 37.455818, 63.688789, 65.662186, 39.418522},
{38.486912, 63.623055, 67.025940, 40.878666, 15.853469, 12.048793, 36.591278, 63.678566, 67.363892, 39.386097, 15.598418, 12.830324, 36.486755, 63.888874, 67.060234, 39.503799}
};

static const float wfolder_overdrive_avg = 36.59637f;

static float cutoff_reso[16][16] = { // k-weigted mean quad
{6.434804, 4.714645, 3.947374, 2.694166, 2.351397, 2.500912, 2.929582, 2.654394, 2.284407, 1.838856, 2.644853, 2.766961, 2.814959, 2.350692, 1.996572, 2.199751},
{6.612917, 5.302139, 3.893952, 2.907703, 2.001597, 2.857677, 2.988061, 3.030843, 2.442840, 2.147922, 2.721878, 2.790063, 2.963842, 2.972988, 2.489201, 2.509848},
{7.350744, 5.508491, 4.067600, 2.890480, 2.176190, 2.566737, 2.546578, 2.596522, 2.278280, 1.956051, 2.581862, 2.548600, 3.036592, 2.538826, 2.439919, 1.898532},
{8.183912, 5.427793, 4.194474, 2.903769, 2.359180, 2.242266, 2.286241, 3.119573, 3.544204, 3.191301, 2.518959, 2.913494, 4.572999, 5.861769, 4.722514, 3.665691},
{7.280022, 4.675759, 3.778736, 3.111181, 2.431638, 2.358587, 2.270216, 2.901183, 3.002531, 2.652288, 1.998837, 2.844437, 3.059708, 3.341244, 2.923321, 2.497309},
{6.960493, 4.442485, 3.891997, 3.066457, 2.455318, 1.855044, 2.421693, 2.687097, 2.769465, 2.300333, 2.023683, 2.392432, 2.613516, 2.961050, 2.950050, 2.489161},
{6.600060, 4.763449, 4.023485, 3.143195, 2.520189, 1.899933, 2.254261, 2.552054, 2.955580, 3.250866, 2.693074, 2.982439, 2.930238, 4.417704, 5.190308, 4.485150},
{5.806700, 5.164190, 3.994584, 3.393530, 2.460938, 2.048136, 2.080137, 2.109953, 2.448718, 2.548533, 2.194179, 2.070688, 3.086098, 4.052840, 4.070950, 3.569471},
{6.268085, 4.558399, 3.507618, 2.848771, 2.446060, 1.948153, 2.073992, 2.101269, 2.966322, 3.262701, 3.049813, 2.411024, 3.578253, 4.676841, 5.917085, 5.060090},
{6.759351, 4.216856, 3.118927, 2.793170, 2.484806, 2.031502, 1.679651, 2.196030, 2.861528, 3.392502, 3.039659, 2.682046, 3.355549, 3.709728, 4.872293, 5.756225},
{6.830225, 4.422283, 3.082938, 2.982032, 2.476420, 2.165540, 1.598023, 2.144882, 2.456352, 2.995122, 2.919489, 2.632485, 3.150194, 3.483125, 5.059742, 6.296432},
{7.835934, 3.491278, 3.291606, 2.806850, 2.564085, 2.023950, 1.767282, 1.784284, 1.954406, 2.394825, 2.952084, 2.615514, 2.811204, 3.472478, 5.163306, 6.343526},
{8.392389, 3.504093, 3.044849, 2.386751, 2.225955, 1.887471, 1.583561, 1.604031, 1.729157, 2.363731, 2.771977, 2.687573, 2.257057, 3.599182, 5.251308, 6.766300},
{8.595600, 3.864176, 2.557690, 1.968429, 1.866560, 1.764059, 1.478550, 1.335737, 1.606975, 2.199224, 2.646084, 2.557677, 2.242978, 3.037829, 3.686971, 5.391070},
{9.063289, 3.624348, 2.342058, 1.823246, 1.851653, 1.604091, 1.497541, 1.124157, 1.500818, 1.905059, 2.288400, 2.217282, 2.097532, 2.477182, 2.971320, 4.915813},
{9.551075, 3.699403, 1.972151, 1.721147, 1.624085, 1.514310, 1.259587, 1.062701, 1.170411, 1.331594, 1.649718, 2.004076, 1.807243, 2.236462, 2.674345, 4.867586},
};

static const float cutoff_reso_avg = 3.19f;

static const float tuning[128] = {
  0.500000f, 0.500000f, 0.500000f, 0.500000f, 0.500000f, 0.529732f, 0.529732f, 0.529732f, 
  0.529732f, 0.529732f, 0.561231f, 0.561231f, 0.561231f, 0.561231f, 0.561231f, 0.594604f, 
  0.594604f, 0.594604f, 0.594604f, 0.594604f, 0.594604f, 0.629961f, 0.629961f, 0.629961f, 
  0.629961f, 0.629961f, 0.667420f, 0.667420f, 0.667420f, 0.667420f, 0.667420f, 0.707107f, 
  0.707107f, 0.707107f, 0.707107f, 0.707107f, 0.749154f, 0.749154f, 0.749154f, 0.749154f, 
  0.749154f, 0.793701f, 0.793701f, 0.793701f, 0.793701f, 0.793701f, 0.840896f, 0.840896f, 
  0.840896f, 0.840896f, 0.840896f, 0.890899f, 0.890899f, 0.890899f, 0.890899f, 0.890899f, 
  0.943874f, 0.943874f, 0.943874f, 0.943874f, 0.943874f, 1.000000f, 1.000000f, 1.000000f, 
  1.000000f, 1.000000f, 1.000000f, 1.059463f, 1.059463f, 1.059463f, 1.059463f, 1.059463f, 
  1.122462f, 1.122462f, 1.122462f, 1.122462f, 1.122462f, 1.189207f, 1.189207f, 1.189207f, 
  1.189207f, 1.189207f, 1.259921f, 1.259921f, 1.259921f, 1.259921f, 1.259921f, 1.334840f, 
  1.334840f, 1.334840f, 1.334840f, 1.334840f, 1.414214f, 1.414214f, 1.414214f, 1.414214f, 
  1.414214f, 1.498307f, 1.498307f, 1.498307f, 1.498307f, 1.498307f, 1.587401f, 1.587401f, 
  1.587401f, 1.587401f, 1.587401f, 1.681793f, 1.681793f, 1.681793f, 1.681793f, 1.681793f, 
  1.681793f, 1.781797f, 1.781797f, 1.781797f, 1.781797f, 1.781797f, 1.887749f, 1.887749f, 
  1.887749f, 1.887749f, 1.887749f, 2.000000f, 2.000000f, 2.000000f, 2.000000f, 2.000000f
};

#endif
