#define PROG_NAME       "ESP32 AcidBox"
#define VERSION         "v.1.2.5b"



#define JUKEBOX                 // real-time endless auto-compose acid tunes
#define JUKEBOX_PLAY_ON_START // should it play on power on, or should it wait for "boot" button to be pressed


//#define USE_INTERNAL_DAC      // use this for testing, SOUND QUALITY SACRIFICED: NOISY 8BIT STEREO
//#define NO_PSRAM              // if you don't have PSRAM on your board, then use this define, but REVERB TO BE SACRIFICED, ONE SMALL DRUM KIT SAMPLES USED 

#define DEBUG_ON              // note that debugging eats ticks initially belonging to real-time tasks, so sound output will be spoild in most cases, turn it off for production build
//#define DEBUG_MASTER_OUT      // serial monitor plotter will draw the output waveform
//#define DEBUG_SAMPLER
//#define DEBUG_SYNTH
//#define DEBUG_JUKEBOX
//#define DEBUG_FX
#define DEBUG_TIMING
//#define DEBUG_MIDI

//#define MIDI_VIA_SERIAL       // use this option to enable Hairless MIDI on Serial port @115200 baud (USB connector), THIS WILL BLOCK SERIAL DEBUGGING as well
#define MIDI_VIA_SERIAL2        // use this option if you want to operate by standard MIDI @31250baud, UART2 (Serial2), 
#define MIDIRX_PIN      4       // this pin is used for input when MIDI_VIA_SERIAL2 defined (note that default pin 17 won't work with PSRAM)
#define MIDITX_PIN      15      // this pin will be used for output (not implemented yet) when MIDI_VIA_SERIAL2 defined

#define POT_NUM 3
#if defined(CONFIG_IDF_TARGET_ESP32S3)
#define I2S_BCLK_PIN    5       // I2S BIT CLOCK pin (BCL BCK CLK)
#define I2S_WCLK_PIN    7       // I2S WORD CLOCK pin (WCK WCL LCK)
#define I2S_DOUT_PIN    6       // to I2S DATA IN pin (DIN D DAT)
const uint8_t POT_PINS[POT_NUM] = {40, 41, 42};
#elif defined(CONFIG_IDF_TARGET_ESP32)
#define I2S_BCLK_PIN    5       // I2S BIT CLOCK pin (BCL BCK CLK)
#define I2S_WCLK_PIN    19      // I2S WORD CLOCK pin (WCK WCL LCK)
#define I2S_DOUT_PIN    18      // to I2S DATA IN pin (DIN D DAT)
const uint8_t POT_PINS[POT_NUM] = {34, 35, 36};
#endif


float bpm = 130.0f;

#define MAX_CUTOFF_FREQ 4000.0f
#define MIN_CUTOFF_FREQ 250.0f

#ifdef USE_INTERNAL_DAC
#define SAMPLE_RATE     22050   // price for increasing this value having NO_PSRAM is less delay time, you won't hear the difference at 8bit/sample
#else
#define SAMPLE_RATE     44100   // 44100 seems to be the right value, 48000 is also OK. Other values are not tested.
#endif

const float DIV_SAMPLE_RATE = 1.0f / (float)SAMPLE_RATE;
const float DIV_2SAMPLE_RATE = 0.5f / (float)SAMPLE_RATE;

#define TABLE_BIT  		        10UL				// bits per index of lookup tables for waveforms, exp(), sin(), cos() etc. 10 bit means 2^10 = 1024 samples
#define TABLE_SIZE            (1<<TABLE_BIT)        // samples used for lookup tables (it works pretty well down to 32 samples due to linear approximation, so listen and free some memory at your choice)
#define TABLE_MASK  	        (TABLE_SIZE-1)        // strip MSB's and remain within our desired range of TABLE_SIZE
#define CICLE_INDEX(i)        (((int32_t)(i)) & TABLE_MASK ) // this way we can operate with periodic functions or waveforms without phase-reset ("if's" are time-consuming)

const float DIV_TABLE_SIZE =  1.0f / (float)TABLE_SIZE;

#define TANH_LOOKUP_MAX 5.0f        // maximum X argument value for tanh(X) lookup table, tanh(X)~=1 if X>4 
const float TANH_LOOKUP_COEF = (float)TABLE_SIZE / TANH_LOOKUP_MAX;
#define DMA_BUF_LEN     32          // there should be no problems with low values, down to 32 samples, 64 seems to be OK with some extra
#define DMA_NUM_BUF     2           // I see no reasom to set more than 2 DMA buffers, but...

const uint32_t DMA_BUF_TIME = (uint32_t)(1000000.0f / (float)SAMPLE_RATE * (float)DMA_BUF_LEN); // microseconds per buffer, used for debugging of time-slots



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
const float DIV_PI = 1.0f/PI;
const float DIV_TWOPI = 1.0f/TWOPI;
#define FORMAT_LITTLEFS_IF_FAILED true

/*
 * TR 808 programs
  1 - Bass Drum 
  2 - Snare Drum 
  3 - Low Tom/Low Conga
  4 - Mid Tom/Mid Conga 
  5 - Hi Tom/Hi Conga 
  6 - Rim Shot/Claves
  7 - hand ClaP/MAracas
  8 - Cow Bell
  9 - CYmbal
  10 - Open Hihat
  11 - Closed Hihat
*/
#define GROUP_HATS  // if so, instruments 06 and 07 will terminate each other (sampler module)
#define OH_NUMBER   // open hat instrument number in kit (for groupping)
#define CH_NUMBER   // closed hat instrument number in kit (for groupping)
#ifdef NO_PSRAM
  #define RAM_SAMPLER_CACHE  40000    // bytes, compact sample set is 132kB, first 8 samples is ~38kB
  #define DEFAULT_DRUMKIT 4           // /data/4/ folder
  #define SAMPLECNT       8           // how many samples we prepare (here just 8)
#else
  #define PRELOAD_ALL                 // allows operating all the samples in realtime
  #define PSRAM_SAMPLER_CACHE 3145728 // bytes, we are going to preload ALL the samples from FLASH to PSRAM
                                      // we divide samples by octaves to use modifiers to particular instruments, not just note numbers
                                      // i.e. we know that all the "C" notes in all octaves are bass drums, and CC_808_BD_TONE affects all BD's
  #define SAMPLECNT       (7 * 12)    // how many samples we prepare (8 octaves by 12 samples)
  #define DEFAULT_DRUMKIT 0           // in my /data /0 has a massive bassdrum , /6 = 808 samples
#endif

#define TINY 0.000001f;

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

 float cutoff_reso[16][16] = {
{4.8385, 4.88747, 4.92047, 4.76436, 4.8962, 5.01568, 4.98983, 5.01559, 5.10654, 5.03281, 5.01903, 4.95362, 4.81538, 4.8074, 4.74791, 4.54329},
{3.87221, 3.90254, 3.82171, 3.75677, 3.7406, 3.67065, 3.69104, 3.56865, 3.54271, 3.67638, 3.65262, 3.57059, 3.58953, 3.51315, 3.45127, 3.37038},
{3.1284, 3.14124, 3.14524, 3.09576, 3.02473, 3.06767, 3.04812, 3.06489, 3.0655, 2.99194, 2.95566, 2.83795, 2.68933, 2.75138, 2.64402, 2.47201},
{2.92398, 2.76649, 2.72365, 2.67662, 2.59435, 2.57171, 2.63079, 2.59806, 2.50945, 2.50648, 2.49537, 2.44069, 2.38885, 2.29797, 2.17515, 2.04705},
{2.69233, 2.58547, 2.62899, 2.56338, 2.56841, 2.48645, 2.40444, 2.36171, 2.26753, 2.17892, 2.22557, 2.13955, 2.05197, 1.9635, 1.877, 1.77883},
{2.49518, 2.41459, 2.35943, 2.48924, 2.4879, 2.30845, 2.28157, 2.2319, 2.19218, 2.12374, 2.04543, 1.96772, 1.87544, 1.72046, 1.71919, 1.58504},
{2.56721, 2.4846, 2.46367, 2.38851, 2.34092, 2.21967, 2.10531, 2.17264, 2.08794, 1.98999, 1.93158, 1.86221, 1.79165, 1.68922, 1.61215, 1.49858},
{2.44912, 2.41703, 2.38924, 2.60044, 2.47826, 2.26369, 2.26848, 2.08206, 2.01731, 1.95231, 1.78664, 1.81617, 1.68899, 1.58835, 1.48491, 1.39299},
{2.42586, 2.44112, 2.3695, 2.38807, 2.42516, 2.21484, 2.30564, 2.09487, 2.14824, 1.97235, 1.88533, 1.7607, 1.67901, 1.56326, 1.41019, 1.35799},
{2.51576, 2.53396, 2.47729, 2.66741, 2.33181, 2.21481, 2.31478, 1.98828, 2.12556, 1.97937, 1.8806, 1.76699, 1.70553, 1.57373, 1.48169, 1.35733},
{2.25554, 2.45139, 2.38947, 2.78224, 2.49177, 2.3555, 2.46807, 2.16115, 2.14116, 1.98996, 1.89712, 1.7226, 1.68581, 1.60263, 1.49875, 1.35428},
{2.51794, 2.50148, 2.46082, 2.69584, 2.30117, 2.27949, 2.5582, 2.17867, 2.31131, 2.21329, 2.02697, 1.9131, 1.75641, 1.61857, 1.52771, 1.35415},
{2.46675, 2.60235, 2.55729, 2.84951, 2.49979, 2.33553, 2.49566, 2.21991, 2.20328, 2.13907, 2.08274, 1.94865, 1.87065, 1.78613, 1.61446, 1.47914},
{2.40512, 2.40291, 2.40859, 2.97096, 2.52717, 2.39973, 2.88218, 2.37344, 2.43893, 2.30513, 2.12342, 1.99408, 1.90687, 1.7411, 1.70994, 1.5803},
{2.54366, 2.64905, 2.52548, 2.75611, 2.52512, 2.28283, 2.65487, 2.36714, 2.51868, 2.44883, 2.36448, 2.20553, 2.13651, 1.99002, 1.77779, 1.66373},
{2.43544, 2.58627, 2.48965, 3.20733, 2.63355, 2.50921, 2.83243, 2.43752, 2.50693, 2.39616, 2.26776, 2.28478, 2.22265, 2.13063, 2.08305, 2.01791}
};

 float wfolder_overdrive[16][16] = {
{1.81553, 2.92774, 4.06149, 5.07091, 6.26805, 7.34979, 8.18204, 8.78785, 9.45271, 10.05631, 10.65088, 11.09989, 11.4481, 11.92296, 12.23205, 12.29738},
{2.6708, 4.41444, 5.95289, 7.47505, 8.39286, 9.33202, 10.16724, 10.7217, 11.29222, 12.31747, 12.93994, 13.2008, 13.66097, 14.02695, 14.27628, 14.42418},
{3.38126, 5.6452, 7.62784, 9.13709, 10.18128, 11.301, 12.05577, 12.97395, 13.60086, 14.02923, 14.41857, 14.53039, 14.12202, 14.99198, 14.77959, 14.68183},
{4.37828, 7.03035, 9.03751, 10.54589, 11.27255, 12.1775, 13.68013, 14.3049, 14.23863, 14.48288, 14.76953, 14.95656, 14.62193, 14.64976, 14.68791, 14.60861},
{6.15714, 9.96185, 13.47305, 14.8845, 14.73223, 14.25685, 14.59641, 15.15803, 15.03315, 13.94896, 14.08422, 13.62272, 13.85014, 14.56888, 15.05049, 14.73128},
{10.18297, 16.0993, 20.0271, 17.58701, 13.204, 9.45962, 11.62678, 15.84615, 18.96596, 17.04844, 14.1457, 11.65228, 12.7452, 15.39917, 18.01944, 16.55685},
{12.50609, 20.47274, 23.53324, 16.68068, 9.79378, 6.98393, 12.2715, 20.1654, 22.88357, 16.51044, 10.072, 7.87354, 13.03754, 20.20183, 22.35985, 16.68395},
{13.70303, 22.44605, 24.62353, 16.09332, 8.1773, 5.98764, 13.66198, 22.47247, 24.37917, 16.06878, 7.99532, 6.49434, 13.79997, 22.42913, 23.63804, 15.76187},
{14.30539, 23.34099, 25.17157, 15.41343, 7.29526, 5.57348, 13.81831, 23.37496, 25.28368, 15.90457, 7.36298, 6.03334, 14.22995, 23.69467, 23.89587, 15.23157},
{14.54421, 23.93977, 25.9227, 15.9401, 6.95805, 5.33766, 14.02165, 23.19047, 25.30687, 15.63357, 6.78814, 5.57623, 14.38434, 24.30402, 25.2735, 15.50198},
{14.32658, 24.08095, 26.12392, 15.84031, 6.72403, 5.14575, 14.19205, 24.49798, 25.84027, 15.76886, 6.62274, 5.35413, 13.98917, 24.50435, 25.33603, 15.37113},
{15.00247, 24.4431, 26.46044, 15.87536, 6.35671, 4.90051, 14.08632, 24.45546, 25.82815, 15.72268, 6.39308, 5.25604, 14.5012, 24.96537, 25.79327, 15.12743},
{14.97389, 24.50721, 26.50383, 15.8626, 6.40081, 4.9022, 14.26976, 24.82402, 25.15142, 15.14676, 6.22857, 5.13013, 14.37914, 24.82676, 25.89133, 15.38875},
{15.2131, 23.8389, 26.23546, 15.77076, 6.33121, 4.79957, 14.31421, 25.04027, 26.28032, 15.6678, 6.1222, 5.11645, 14.19984, 24.30534, 25.7695, 15.35306},
{15.17793, 24.84065, 26.53333, 15.91002, 6.31883, 4.62659, 13.85447, 24.78054, 25.97502, 15.46608, 6.03461, 5.06064, 14.63126, 25.18358, 26.04326, 15.42947},
{15.16892, 24.67783, 26.40122, 15.80733, 6.27843, 4.76886, 14.31771, 25.04004, 26.3323, 15.22194, 5.83839, 4.99684, 14.46052, 25.16456, 25.8343, 15.38862}
};
