#ifdef JUKEBOX
// This is The "Endless Acid Banger"
//
// Pattern generator code taken from
//    https://www.vitling.xyz/toys/acid-banger/
// created by Vitling (David Whiting) i.am@thewit.ch
//
// The rest of the code by written by thement@ibawizard.net.
// Some related info probably at http://tips.ibawizard.net/
//
// This work is licensed under a Creative Commons Attribution 4.0 International License.
//
//
// What is this?
// =============
// The endless acid banger is autonomous pattern generator that spits acid-like
// note and drum sequences via MIDI.
//
// How to make this work?
// ======================
// Attach 10 buttons to Arduino (pinout specified bellow), connect serial TX
// to DIN-5 female socket (TX to pin 3 via 220 Ohm resistor, 5V to pin 4 via
// 220 Ohm resistor, GND to pin 2) and let it play.
//
// How does the interface work?
// ============================
// - "Play" button starts/stops the sequencer
// - "Gen synth1" button generates new melody for synth 1 from current note set
// - "Gen synth2" button ditto for synth 1
// - "Gen notes" creates new note set and regenerates the synth1/synth2
//   melodies from it
// - "Gen drum" generates new drum pattern
// - "Mem1-5" buttons switch to different patterns. If you hold one memory and
//   then press another one, the first one gets copied to the second.
//
// Have fun!
//
// ===============================
// 2023 edit by Copych
// added auto CC ramps for reso, cutoff etc.
// added drum parts categorizing
// added auto breaks/fills 
// added crash cymbal
// removed/modified buttons processing
//

#define KICK_NOTE               0 //001
#define SNARE_NOTE              1 //002
#define CLOSED_HAT_NOTE         CH_NUMBER //007
#define OPEN_HAT_NOTE           OH_NUMBER //008
#define PERCUSSION_NOTE         11 //005
#define CRASH_NOTE              9 //010

// Pin numbers to which are buttons attached (connect one side of button to pin, the other to ground)
#define GEN_SYNTH1_BUTTON_PIN   23
#define GEN_SYNTH2_BUTTON_PIN   23
#define GEN_NOTES_BUTTON        23
#define GEN_DRUM_BUTTON         23
#define PLAY_BUTTON             0
#define MEM1_BUTTON             23
#define MEM2_BUTTON             23
#define MEM3_BUTTON             23
#define MEM4_BUTTON             23
#define MEM5_BUTTON             23

#define send_midi_start() {}
#define send_midi_stop()  {}
#define send_midi_tick() {}
 

#define NUM_RAMPS 6           // simultaneous knob rotatings
#ifndef NO_PSRAM
  #define NUM_SYNTH_CCS 12    // how many synth CC params do we have to play
  #define NUM_DRUM_CCS  7     // how many drum CC params do we have to play
  #define VOL_SYNTH1     100
  #define VOL_SYNTH2     100
  #define VOL_DRUMS     100
#else
  #define NUM_SYNTH_CCS 11    // how many synth CC params do we have to play
  #define NUM_DRUM_CCS  5     // how many drum CC params do we have to play
  #define VOL_SYNTH1     100
  #define VOL_SYNTH2     100
  #define VOL_DRUMS     100

#endif

uint8_t current_drumkit = (DEFAULT_DRUMKIT*12); // offset for drum note numbers (instruments are groupped by 12)

struct sSynthCCs {
  uint8_t cc_number;
  uint8_t cc_couple;
  uint8_t cc_default_value;
  uint8_t cc_min_value;
  uint8_t cc_max_value;
  bool reset_after_use;
};

sSynthCCs synth1_ramps[NUM_SYNTH_CCS] = {
  //cc                 cpl             def   min max   reset
#ifndef NO_PSRAM
  {CC_303_REVERB_SEND, 0,              5,    2,  127,  true},
#endif
  {CC_303_PAN,        0,              47,   0,  127,  true},
  {CC_303_WAVEFORM,   0,              0,    0,  64,   true}, // SQUARE
  {CC_303_RESO,       CC_303_CUTOFF,  30,   40, 125,  true},
  {CC_303_CUTOFF,     CC_303_RESO,    60,   0,  127,  true},
  {CC_303_DECAY,      0,              20,   15, 64,  true},
  {CC_303_ATTACK,     0,              1,    3,  10,   true},
  {CC_303_ENVMOD_LVL, 0,              100,  0,  127,  false},
  {CC_303_ACCENT_LVL, 0,              10,   0,  70,  true},
  {CC_303_DELAY_SEND, 0,              0,    64, 127,  false},
  {CC_303_DISTORTION, 0,              0,    0,  100,  true},
  {CC_303_OVERDRIVE,  0,              5,    0,  50,  true}
};

sSynthCCs synth2_ramps[NUM_SYNTH_CCS] = {
  //cc                 cpl             def   min max   reset
#ifndef NO_PSRAM
  {CC_303_REVERB_SEND, 0,              5,    2,  127,  true},
#endif
  {CC_303_RESO,       CC_303_CUTOFF,  64,   60, 127,  true},
  {CC_303_CUTOFF,     CC_303_RESO,    20,   0,  100,  false},
  {CC_303_PAN,        0,              80,   0,  127,  true},
  {CC_303_ENVMOD_LVL, 0,              100,  15, 127,  false},
  {CC_303_WAVEFORM,   0,              127,  64, 127,  true}, // SAW
  {CC_303_DELAY_SEND, 0,              0,    64, 127,  false},
  {CC_303_ACCENT_LVL, 0,              0,   0, 127,  true},
  {CC_303_DECAY,      0,              20,   15, 64,  true},
  {CC_303_ATTACK,     0,              1,    3,  15,   true},
  {CC_303_OVERDRIVE,  0,              1,    0,  10,  true},
  {CC_303_DISTORTION, 0,              20,    0,  127,  true}
};

sSynthCCs drum_ramps[NUM_DRUM_CCS] = {
  //cc                  cpl def   min max   reset
#ifndef NO_PSRAM
  {CC_808_REVERB_SEND,  0,  5,    30,  127, true},  // reverb is not available with no psram
  {CC_808_DELAY_SEND,   0,  0,    64,  127, true}, // delay for drums needs more delay time (read 'RAM') than we can afford
#endif
  {CC_808_CUTOFF,       0,  64,   64,  127, true},
  {CC_808_RESO,         0,  64,   0,   127, true},
  {CC_808_SD_TONE,      0,  64,   64,  127, true},
  {CC_808_BD_DECAY,     0,  127,  50,  127, true},
  {CC_808_BD_TONE,      0,  64,   40,  64,  true}
};

struct sMidiRamp {
  uint8_t chan = 0;
  uint8_t cc_number = 0;
  float value = 0.0f;
  float min_val = 0.0f;
  float max_val = 127.0f;
  float def_val = 64.0f;
  bool need_reset = false;
  float stepPer16th = 0.0f;
  int16_t leftBars = 0;
} midiRamps[NUM_RAMPS];


typedef enum drum_kinds {
  DrumBreak,
  DrumStraight,
  DrumHang,
  DrumAny,
  DrumNone
} drum_kinds ;

enum {
  KickElectro,
  KickFourFloor,
  KickBigbeat,
  KickNone,
  /* anything bellow this line will never be picked */
};

enum {
  SnareBackbeat,
  SnareSkip,
  SnareFill,
  SnareBreak,
  SnareStraight,
  SnareNone,
  /* anything bellow this line will never be picked */
};

enum {
  HatsOffbeats,
  HatsClosed,
  HatsPop,
  HatsPat1,
  HatsNone,
  /* anything bellow this line will never be picked */
};

enum {
  PercFiller,
  PercXor1,
  PercXor2,
  PercEcho,
  PercRolls,
  PercNone,
  /* anything bellow this line will never be picked */
};

enum {
  NumInstruments = 2 + 6,
  AccentedMidiVol = 120,
  NormalMidiVol = 70,
};

enum {
  NumMemories = 5,
  MaxNoteSet = 16,
  PatternLength = 16,
};

enum {
  ButPat1 = 0,
  ButNotes = 2,
  ButDrums = 3,
  ButPlay = 4,
  ButMem1 = 5,
  ButLast = ButMem1 + NumMemories,
};

typedef struct Pattern Pattern;
typedef struct Memory Memory;
typedef struct Instrument Instrument;
typedef enum  {sIdle, sPlaying} bStatus;


struct Button {
  uint8_t history;
  byte pin;
  uint8_t numb;
};

struct Instrument {
  // 1-base indexed MIDI channel (first channel is 1)
  byte midi_channel;
  byte is_drum, drum_note;
  void (*noteon)(byte chan, byte note, byte vel);
  void (*noteoff)(byte chan, byte note);
  byte playing_note;
};

static Instrument instruments[NumInstruments];

static uint32_t bar_current = 0; // it counts bars


struct sBreak {
  bStatus status = sIdle;
  byte length = 0;
  uint32_t start = 0;
  uint32_t after = 0;
} Break;

struct Pattern {
  uint16_t accent, glide;
  uint8_t notes[PatternLength];
};

struct Memory {
  Pattern patterns[NumInstruments];
  byte note_set[MaxNoteSet];
  byte num_notes_in_set;
  uint16_t random_seed;
};

static Memory memories[NumMemories];
static byte cur_memory;


#define is_pressed(x) (buttons[x].history == 0)
#define just_pressed(x) (buttons[x].history == 0x80)

static struct Button buttons[ButLast];
static byte button_divider;
static unsigned long now;
static unsigned long last_midi_tick;

static const byte button_pins[ButLast] = {
  GEN_SYNTH1_BUTTON_PIN,
  GEN_SYNTH2_BUTTON_PIN,
  GEN_NOTES_BUTTON,
  GEN_DRUM_BUTTON,
  PLAY_BUTTON,
  MEM1_BUTTON,
  MEM2_BUTTON,
  MEM3_BUTTON,
  MEM4_BUTTON,
  MEM5_BUTTON,
};


static void send_midi_noteon(byte chan, byte note, byte vol) {
#ifdef ENABLE_MIDI_OUT
  #ifdef MIDI_VIA_SERIAL  
    MIDI.sendNoteOn(note, vol, chan);
  #endif
  #ifdef MIDI_VIA_SERIAL2  
    MIDI2.sendNoteOn(note, vol, chan);
  #endif
#endif
  handleNoteOn( chan, note, vol) ;
}

static void send_midi_noteoff(byte chan, byte note) {
#ifdef ENABLE_MIDI_OUT
  #ifdef MIDI_VIA_SERIAL  
    MIDI.sendNoteOn(note, 0, chan);
  #endif
  #ifdef MIDI_VIA_SERIAL2  
    MIDI2.sendNoteOn(note, 0, chan);
  #endif
#endif
  handleNoteOff( chan, note, 0) ;
}

static void init_midi() {
  //  Serial.begin(115200);
  //  MIDI.begin(MIDI_CHANNEL_OMNI);
  pinMode(LED_BUILTIN, OUTPUT);
  for (int i = 0; i < ButLast; i++) {
    init_button(&buttons[i], button_pins[i], i + 1 );
  }
  init_instruments();
  init_patterns();
#ifdef JUKEBOX_PLAY_ON_START
  do_midi_start();
#endif
}
static void send_midi_control(byte chan, byte cc_number, byte cc_value) {
  //MIDI.sendControlChange (  cc_number,  cc_value,  chan);
  handleCC( chan,  cc_number,  cc_value);
}

/*
   Pseudo-random generator with restorable state
*/

static inline uint16_t lfsr16_next(uint16_t x)
{
  uint16_t y = x >> 1;
  if (x & 1)
    y ^= 0xb400;
  return y;
}

union seeder_t {
  double dseed= 0.9876543021203450678091203456/random(3,0xffff);
  struct{
    uint8_t x;
    uint16_t useed ;

  };
} seeder ;
//static uint16_t myRandomState = 0x1234; 
static uint16_t myRandomState = seeder.useed ;

static uint16_t myRandomAddEntropy(uint16_t data) {
  myRandomState = lfsr16_next((myRandomState << 1) ^ data);
  return myRandomState;
}

static uint16_t myRandomRaw() {
  myRandomState = lfsr16_next(myRandomState);
  return myRandomState;
}

static inline uint16_t myRandom(uint16_t max) {
  if (max==0) return 0;
  return myRandomRaw() % max;
}

/*
   Buttons
*/

static void read_button(struct Button *button)
{
  if (button->numb == 5) { // start/stop is a real button "boot" ( GPIO0 )
    button->history = (button->history << 1) | (digitalRead(button->pin) == HIGH);
  } else {
    button->history = 0;
  }
}

static void init_button(struct Button *button, byte pin, uint8_t num)
{
  button->history = 0xff;
  button->pin = pin;
  button->numb = num;
  pinMode(pin, INPUT_PULLUP);
}

/*
   Instruments
*/

static void instr_noteoff(byte instr) {
  Instrument *ins = &instruments[instr];

  if (ins->playing_note != 0) {
    if (ins->noteoff != NULL)
      ins->noteoff(ins->midi_channel, ins->playing_note);
    ins->playing_note = 0;
  }
}

static void instr_allnotesoff() {
  for (int i = 0; i < NumInstruments; i++) {
    instr_noteoff(i);
  }
}

static void instr_noteon_raw(byte instr, byte note, byte vol, byte do_glide) {
  Instrument *ins = &instruments[instr];

  // All instruments are monophonic, so noteoff before noteon
  if (ins->playing_note != 0) {
    if (do_glide && !ins->is_drum) {
      // Implement glide by playing two notes at once. Also known as "fingered glide"
      if (ins->noteon != NULL)
        ins->noteon(ins->midi_channel, note, vol);
      instr_noteoff(instr);
      ins->playing_note = note;
      return;
    }
    instr_noteoff(instr);
  }
  if (ins->noteon != NULL)
    ins->noteon(ins->midi_channel, note, vol);
  ins->playing_note = note;
}

static void instr_noteon(byte instr, byte value, byte do_glide, byte do_accent) {
  Instrument *ins = &instruments[instr];
#ifdef DEBUG_JUKEBOX_
  DEBF("glide=%d accent=%d\r\n", do_glide, do_accent);
#endif
  if (ins->is_drum) {
    // For drums: value is volume, accent and glide are ignored
    instr_noteon_raw(instr, current_drumkit + ins->drum_note, value, 0);
  } else {
    // For non-drums: value is note, volume is accent, glide is used
    instr_noteon_raw(instr, value, do_accent ? AccentedMidiVol : NormalMidiVol, do_glide);
  }
}

/*
   Sequencer
*/

void sequencer_step(byte step) {
#ifdef MIDI_RAMPS
  do_midi_ramps();
#endif
#ifdef DEBUG_JUKEBOX_
  DEBF("midi step %d\r\n", step);
#endif
  // Play all notes in current step
  for (int i = 0; i < NumInstruments; i++) {
    Pattern *pat = &memories[cur_memory].patterns[i];
    byte accent = (pat->accent >> step) & 1;
    byte glide = (pat->glide >> step) & 1;
    byte value = pat->notes[step];

    if (value > 0)
      instr_noteon(i, value, glide, accent);
    else
      instr_noteoff(i);
  }
  if (Break.after == bar_current && step == 0) {
    instr_noteon(NumInstruments - 1, 127, 0, 0);
    //    instr_noteon_raw(NumInstruments-1, CRASH_NOTE, 127, 0);
#ifdef PRELOAD_ALL
    if (flip(30)) {
      //change drumkit
      current_drumkit = myRandom(((Drums.GetSamplesCount()-1)/12)) * 12 ;
      Drums.SetProgram(current_drumkit);
#ifdef DEBUG_JUKEBOX
      DEBF("Selected drumkit: %d\r\n" , current_drumkit);
#endif
#endif
    }
#ifdef DEBUG_JUKEBOX
    DEBUG("CRASH!!!!!!!!!!!!!!!!!!!!!");
#endif
#ifdef MIDI_RAMPS
    check_midi_ramps(true);
#endif
  }
  
#ifdef FLASH_LED
  if (step % 4 == 0 || step == 1) {
    digitalWrite(LED_BUILTIN, HIGH);
    #ifdef LOLIN_RGB
      pixels.setPixelColor(0, pixels.Color(rand() % 32, rand() % 32, rand() % 32));
      pixels.show();
    #endif
  }  else {
    digitalWrite(LED_BUILTIN, LOW);
    #ifdef LOLIN_RGB
      pixels.setPixelColor(0, pixels.Color(0,0,0));
      pixels.show();
    #endif
  }
#endif
}

/*
   Endless Acid Banger pattern generator

   Adapted from https://www.vitling.xyz/toys/acid-banger/
   created by Vitling (David Whiting) i.am@thewit.ch
*/

#define NOTE_LIST(x...) (int8_t[]) { x, -1 }
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const int8_t *const offset_choices[] = {
  NOTE_LIST(0, 0, 12, 24, 27),
  NOTE_LIST(0, 0, 0, 12, 10, 19, 26, 27),
  NOTE_LIST(0, 0, 0, 1, 7, 10, 12, 13),
  NOTE_LIST(0),
  NOTE_LIST(0, 0, 0, 0, 0, 0, 1, 13, 25),
  NOTE_LIST(0, 0, 0, 12, 24),
  NOTE_LIST(0, 0, 12, 12, 18, 24, 24),
  NOTE_LIST(0, 0, 7, 14, 24, 24),
  NOTE_LIST(0, 0, 12, 14, 15, 19),
  NOTE_LIST(0, 0, 0, 12, 12, 13, 16, 19, 22, 24, 25),
  NOTE_LIST(0, 0, 0, 7, 12, 15, 17, 20, 24),
};

static byte generate_note_set(uint8_t *note_set) {
  // Random root note
  byte root = myRandom(15) + 28;

  // Random note set
  byte set = myRandom(ARRAY_SIZE(offset_choices));

  // Copy notes from note set and offset them by "root note"
  int i;
  for (i = 0; i < MaxNoteSet; i++) {
    int8_t note = offset_choices[set][i];
    if (note < 0)
      break;
    *note_set++ = root + note;
  }

  return i;
}

// Flip a coin
static byte flip(byte percent_chance) {
  return myRandom(100) < percent_chance;
}

static void generate_melody(uint8_t *note_set, byte note_set_len,
                            uint8_t *pattern, byte pattern_len,
                            uint16_t *accent, uint16_t *glide) {

  uint8_t density = 255;

  *accent = 0;
  *glide = 0;
  for (int i = 0; i < pattern_len; i++) {
//    uint8_t chance = ((uint16_t) density * (i % 4 == 0 ?  60 : (i % 3 == 0 ?  50 : (i % 2 == 0 ?  30 : 10)))) >> 8;
    uint8_t chance = ((uint16_t) density * (i % 4 == 0 ?  90 : (i % 3 == 0 ?  80 : (i % 2 == 0 ?  50 : 10)))) >> 8;
    if (flip(chance)) {
      pattern[i] = note_set[myRandom(note_set_len)];
      if (flip(30))
        *accent |= 1u << i;
      if (flip(70))
        *glide |= 1u << i;
    } else {
      pattern[i] = 0;
    }
  }
}

static void generate_drums(byte *kick, byte *snare, byte *oh, byte *ch, byte *perc, byte *crash, drum_kinds drum_kind );

static void generate_drums(byte *kick, byte *snare, byte *oh, byte *ch, byte *perc, byte *crash, drum_kinds drum_kind ) {
  memset(kick,  0, PatternLength); // zero patterns
  memset(snare, 0, PatternLength);
  memset(oh,    0, PatternLength);
  memset(ch,    0, PatternLength);
  memset(perc,  0, PatternLength);

  byte kick_mode =  KickNone;
  byte hat_mode =   HatsNone;
  byte snare_mode = SnareNone;
  byte perc_mode =  PercNone;

  byte rndVal ;
  switch (drum_kind) {
    case DrumBreak:
      rndVal = myRandom(100);
      if (rndVal < 10) {
        kick_mode = KickBigbeat;
      } else if (rndVal < 60) {
        kick_mode = KickFourFloor;
      } else {
        kick_mode = KickNone;
      }

      rndVal = myRandom(100);
      if (rndVal < 40) {
        snare_mode = SnareFill;
      } else if (rndVal < 80) {
        snare_mode = SnareBreak;
      } else {
        snare_mode = SnareBackbeat;
      }

      hat_mode = myRandom(HatsNone);
      perc_mode = myRandom(PercNone);
      break;
    case DrumStraight:
      rndVal = myRandom(100);
      if (rndVal < 20) {
        kick_mode = KickBigbeat;
      } else {
        kick_mode = KickFourFloor;
      }

      rndVal = myRandom(100);
      if (rndVal < 60) {
        snare_mode = SnareStraight;
      } else {
        snare_mode = SnareBackbeat;
      }

      if (flip(70)) hat_mode = HatsPop;
      else hat_mode = myRandom(HatsNone);

      perc_mode = myRandom(PercNone);
      break;
    case DrumHang:
      rndVal = myRandom(100);
      if (rndVal < 50) {
        kick_mode = KickBigbeat;
      } else {
        kick_mode = KickNone;
      }

      rndVal = myRandom(100);
      if (rndVal < 60) {
        snare_mode = SnareStraight;
      } else {
        snare_mode = SnareBackbeat;
      }

      hat_mode = myRandom(HatsNone);
      perc_mode = myRandom(PercNone);
      break;
    case DrumNone:
      // nothing
      break;
    case DrumAny:
    default:
      kick_mode = myRandom(KickNone);
      hat_mode = myRandom(HatsNone);
      snare_mode = myRandom(SnareNone);
      perc_mode = myRandom(PercNone);
  }


  if (kick_mode == KickFourFloor) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 4 == 0)
        kick[i] = 120;
    }
  } else if (kick_mode == KickElectro) {
    for (int i = 0; i < PatternLength; i++) {
      if (i == 0)
        kick[i] = 127;
      else if (i % 2 == 0 && i % 8 != 4 && flip(50))
        kick[i] = myRandom(110);
      else if (flip(5))
        kick[i] = myRandom(110);
    }
  } else if (kick_mode == KickBigbeat) {
    for (int i = 0; i < PatternLength; i++) {
      if (i == 0)
        kick[i] = 127;
      else if (i == 14 && flip(20))
        kick[i] = myRandom(80);
    }
  }

  if (snare_mode == SnareBackbeat) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 8 == 0)
        snare[i] = 110;
    }
  } else if (snare_mode == SnareFill) {
    for (int i = 0; i < PatternLength; i++) {
      snare[i] = 120;
    }
  } else if (snare_mode == SnareStraight) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 8 == 4)
        snare[i] = 80;
    }
  } else if (snare_mode == SnareBreak) {
    for (int i = 0; i < PatternLength; i++) {
      switch (i) {
        case 2:
        case 8:
          break;
        case 4:
        case 5:
        case 6:
        case 10:
          snare[i] = myRandom(100);
          break;
        default:
          snare[i] = 120;
      }
    }
  }  else if (snare_mode == SnareSkip) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 8 == 3 || i % 8 == 6)
        snare[i] = 90 + myRandom(37);
      else if (i % 2 == 0 && flip(20))
        snare[i] = 40 + myRandom(25);
      else if (flip(10))
        snare[i] = 25 + myRandom(25);
    }
  }

  if (hat_mode == HatsOffbeats) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 4 == 2)
        oh[i] = 50;
      else if (flip(30)) {
        if (flip(50))
          ch[i] = myRandom(25);
        else
          oh[i] = myRandom(25);
      }
    }
  } else if (hat_mode == HatsClosed) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 2 == 0)
        ch[i] = 50;
      else if (flip(50))
        ch[i] = myRandom(40);
    }
  } else if (hat_mode == HatsPop) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 4 == 2)
        oh[i] = 60;
      else {
        ch[i] = 40 + myRandom(40);
      }
    }
  } else if (hat_mode == HatsPat1) {
    for (int i = 0; i < PatternLength; i++) {
      if (i % 8 != 1 && i % 8 != 4 && i % 8 != 7 )
        ch[i] = 80;
    }
  }

  if (perc_mode == PercFiller) {
    for (int i = 0; i < PatternLength; i++) {
      if (oh[i] == 0 && ch[i] == 0 && kick[i] == 0 && snare[i] == 0)
        perc[i] = 50 + myRandom(37);
    }
  } else if (perc_mode == PercXor1) {
    for (int i = 0; i < PatternLength; i++) {
      if ((kick[i] == 0) ^ (snare[i] == 0))
        perc[i] = 50 + myRandom(37);
    }
  } else if (perc_mode == PercXor2) {
    for (int i = 0; i < PatternLength; i++) {
      if ((oh[i] == 0) ^ (ch[i] == 0))
        perc[i] = 50 + myRandom(37);
    }
  } else if (perc_mode == PercEcho) {
    byte distance = 1 + myRandom(7);

    for (int i = 0; i < PatternLength; i++) {
      byte prev_step = (i + PatternLength - distance) % PatternLength;
      if (ch[prev_step] || oh[prev_step])
        perc[i] = 50 + myRandom(37);
    }
  } else if (perc_mode == PercRolls) {
    byte roll = 0, roll_vol = 0;
    for (int i = 0; i < PatternLength; i++) {
      byte do_roll = 0;
      if (i % 8 == 3 && flip(40))
        do_roll = 1;
      else if (i % 2 == 0 && flip(20))
        do_roll = 1;
      else if (flip(10))
        do_roll = 1;

      if (do_roll) {
        roll_vol = 50 + myRandom(37);
        roll = 4;
      }

      if (roll > 0) {
        perc[i] = roll_vol;
        roll_vol /= 2;
        roll--;
      }
    }
  }
}

/*
   Generator-to-pattern binding
*/

void mem_generate_melody(byte mem, byte voice) {
  Memory *m = &memories[mem];
  Pattern *p = &m->patterns[voice];
  // Temporarily change random seed to a pre-defined state so that we can generate
  // identical melody.
  uint16_t random_state = myRandomState;
  myRandomState = (m->random_seed << 1) ^ voice;
#ifdef DEBUG_JUKEBOX_
  DEBF("generating %d/%d with seed %u \r\n", mem, voice, myRandomState);
#endif
  generate_melody(
    m->note_set, m->num_notes_in_set,
    p->notes, sizeof(p->notes),
    &p->accent, &p->glide);
  myRandomState = random_state;
}

void mem_generate_melody_and_seed(byte mem, byte voice) {
  Memory *m = &memories[mem];

  m->random_seed = myRandomRaw();
  mem_generate_melody(mem, voice);
}

void mem_generate_note_set(byte mem) {
  Memory *m = &memories[mem];
  m->num_notes_in_set = generate_note_set(m->note_set);
  for (int i = 0; i < 2; i++)
    mem_generate_melody(mem, i);
}

void mem_generate_drums(byte mem, enum drum_kinds drum_kind);
void mem_generate_drums(byte mem, enum drum_kinds drum_kind) {
  Memory *m = &memories[mem];
  generate_drums(
    m->patterns[2].notes,
    m->patterns[3].notes,
    m->patterns[5].notes,
    m->patterns[4].notes,
    m->patterns[6].notes,
    m->patterns[7].notes,
    drum_kind);
}

void mem_generate_all(byte mem) {
  mem_generate_note_set(mem);
  mem_generate_drums(mem, DrumStraight);
  for (int i = 0; i < 2; i++)
    mem_generate_melody_and_seed(mem, i);
}

void print_pattern(struct Pattern *p, byte is_drum) {
#ifdef DEBUG_JUKEBOX
  for (int i = 0; i < PatternLength; i++)
    DEBF("%3d ", p->notes[i]);

  if (!is_drum) {
    for (int i = 0; i < PatternLength; i++)
      DEBF(" %c%c \r\n", (p->accent & (1u << i)) ? 'A' : ' ', (p->glide & (1u << i)) ? '~' : ' ');
  }
#endif
}

void print_memory(byte mem) {
  Memory *m = &memories[mem];
#ifdef DEBUG_JUKEBOX
  DEBF("--- memory %d ---", mem);
  DEBF("noteset[%d]:", m->num_notes_in_set);
#endif
  for (int i = 0; i < m->num_notes_in_set; i++)
#ifdef DEBUG_JUKEBOX
    DEBF(" %d", m->note_set[i]);
#endif
  for (int i = 0; i < NumInstruments; i++)
    print_pattern(&m->patterns[i], instruments[i].is_drum);
}

void init_patterns() {
  for (int i = 0; i < NumMemories; i++)
    mem_generate_all(i);
}

/*
   MIDI clock
*/

#define MIDI_TICKS_PER_16TH 1

static byte midi_playing, midi_tick, midi_step;
const float tick_coef = 1000ul * 15 / MIDI_TICKS_PER_16TH;
static unsigned long midi_tick_ms = tick_coef / bpm;

inline void set_bpm(float newBpm) {
  bpm = newBpm;
  midi_tick_ms = tick_coef / newBpm;
}

static void decide_on_break() {
  uint32_t bars_played = bar_current - Break.after ;
  if ( Break.status == sIdle ) {
    // plan a break ?
    if ( bars_played == 28 ) {
      // 100% 1-bar break
      Break.status = sPlaying;
      Break.start = bar_current  ;
      Break.length = 4;
      Break.after = Break.start + Break.length;
    } else if ( bars_played == 15 ) {
      // 50% 1-bar break
      if (flip(20)) {
        Break.status = sPlaying;
        Break.start = bar_current  ;
        Break.length = 1;
        Break.after = Break.start + Break.length;
      }
    } else if (bars_played == 14) {
      if (flip(15)) {
        // 2-bars break
        Break.status = sPlaying;
        Break.start = bar_current  ;
        Break.length = 2;
        Break.after = Break.start + Break.length;
      }
    } else if (bars_played == 13) {
      if (flip(15)) {
        // 3-bars break
        Break.status = sPlaying;
        Break.start = bar_current  ;
        Break.length = 3;
        Break.after = Break.start + Break.length;
      }
    } else if (bars_played == 12) {
      if (flip(15)) {
        // 4-bars break
        Break.status = sPlaying;
        Break.start = bar_current  ;
        Break.length = 4;
        Break.after = Break.start + Break.length;
      }
    } else if (bars_played == 7) {
      if (flip(15)) {
        // 1-bar break
        Break.status = sPlaying;
        Break.start = bar_current ;
        Break.length = 1;
        Break.after = Break.start + Break.length;
      }
    } else if (bars_played == 6) {
      if (flip(15)) {
        // 2-bars break
        Break.status = sPlaying;
        Break.start = bar_current  ;
        Break.length = 2;
        Break.after = Break.start + Break.length;
      }
    } else if (bars_played == 3) {
      if (flip(10)) {
        // 1-bar break
        Break.status = sPlaying;
        Break.start = bar_current ;
        Break.length = 1;
        Break.after = Break.start + Break.length;
      }
    }
    if (Break.start == bar_current ) mem_generate_drums(cur_memory, DrumBreak);
  } else { // Break.status != sIdle
    if (Break.after == bar_current) {
      Break.status = sIdle;
      mem_generate_drums(cur_memory, DrumStraight);
      if (flip(10)) mem_generate_drums(cur_memory, DrumHang);
      if (flip(80)) mem_generate_melody_and_seed(cur_memory, 0);
      if (flip(60)) mem_generate_melody_and_seed(cur_memory, 1);
      if (flip(15)) mem_generate_note_set(cur_memory);
    }
  }// Break.status ?? sIdle ?
#ifdef DEBUG_JUKEBOX
  DEBF("bar=%d  break.len=%d  break.status=%d \r\n", bars_played, Break.length, Break.status);
#endif
}

static void do_midi_start() {
  midi_playing = 1;
  midi_tick = MIDI_TICKS_PER_16TH - 1;
  midi_step = -1;
  send_midi_control(SYNTH1_MIDI_CHAN, 10, 10);
  send_midi_control(SYNTH2_MIDI_CHAN, 10, 117);
  send_midi_control(SYNTH1_MIDI_CHAN, 74, 64);
  send_midi_control(SYNTH2_MIDI_CHAN, 74, 40);
  send_midi_control(SYNTH1_MIDI_CHAN, 70, 127); // saw
  send_midi_control(SYNTH2_MIDI_CHAN, 70, 0);   // square
  send_midi_control(SYNTH1_MIDI_CHAN, 71, 100);
  send_midi_control(SYNTH2_MIDI_CHAN, 71, 100);
  send_midi_control(SYNTH1_MIDI_CHAN, 72, 30);
  send_midi_control(SYNTH2_MIDI_CHAN, 72, 30);
  send_midi_control(SYNTH1_MIDI_CHAN, 73, 3);
  send_midi_control(SYNTH2_MIDI_CHAN, 73, 3);
  send_midi_control(SYNTH1_MIDI_CHAN, 91, 5);  // reverb send
  send_midi_control(SYNTH2_MIDI_CHAN, 91, 5);  // reverb send
  send_midi_control(DRUM_MIDI_CHAN,   91, 4);  // reverb send
  send_midi_control(SYNTH1_MIDI_CHAN, 7, VOL_SYNTH1);
  send_midi_control(SYNTH2_MIDI_CHAN, 7, VOL_SYNTH2);
  send_midi_control(DRUM_MIDI_CHAN,   7, VOL_DRUMS);
  send_midi_control(DRUM_MIDI_CHAN,   87, 127); // reverb time
  send_midi_control(SYNTH1_MIDI_CHAN, 94, 3);  // post-overdrive
  send_midi_control(SYNTH2_MIDI_CHAN, 94, 2);  // post-overdrive
  send_midi_control(SYNTH1_MIDI_CHAN, 93, 10); // compressor ratio
  send_midi_start();
}

static bool ramp_cc_repeated(uint8_t test_cc, uint8_t chan) {
  bool repeated = false ;
  for (int i = 0; i < NUM_RAMPS; i++) {
    if (midiRamps[i].cc_number == test_cc && midiRamps[i].chan == chan) repeated = true;
  }
  return repeated;
}

static void do_midi_ramps() {
  for (int i = 0; i < NUM_RAMPS; i++) {
    midiRamps[i].value += midiRamps[i].stepPer16th;
    if (midiRamps[i].value >= midiRamps[i].max_val ) {
      midiRamps[i].value = midiRamps[i].max_val;
      midiRamps[i].stepPer16th = -midiRamps[i].stepPer16th;
    } else if (midiRamps[i].value <= midiRamps[i].min_val ) {
      midiRamps[i].value = midiRamps[i].min_val;
      midiRamps[i].stepPer16th = -midiRamps[i].stepPer16th;
    }
    uint8_t val = (uint8_t)(midiRamps[i].value);
    send_midi_control(midiRamps[i].chan,  midiRamps[i].cc_number, val);
#ifdef DEBUG_JUKEBOX
    DEBF("ramp: %d  chan: %d  cc: %d = %d \r\n", i, midiRamps[i].chan, midiRamps[i].cc_number, val);
#endif
  }
}

static void check_midi_ramps(boolean force_restart) {
  if (Break.status == sIdle || force_restart) {
    for (int i = 0; i < NUM_RAMPS; i++) {
      midiRamps[i].leftBars--;
      if (midiRamps[i].leftBars <= 0 || force_restart) { // no more bars left for the ramp
        if (midiRamps[i].need_reset) {
          send_midi_control(midiRamps[i].chan, midiRamps[i].cc_number, midiRamps[i].def_val);
        }
        uint8_t chanSeed = random(0, 100); // probability
        uint8_t ccSeed;
        if (chanSeed < 45) {
          do {
            ccSeed = random(0, NUM_SYNTH_CCS);
          } while (ramp_cc_repeated(synth1_ramps[ccSeed].cc_number, SYNTH1_MIDI_CHAN));
          midiRamps[i].chan = SYNTH1_MIDI_CHAN;
          midiRamps[i].cc_number = synth1_ramps[ccSeed].cc_number;
          midiRamps[i].min_val = synth1_ramps[ccSeed].cc_min_value;
          midiRamps[i].max_val = synth1_ramps[ccSeed].cc_max_value;
          midiRamps[i].def_val = synth1_ramps[ccSeed].cc_default_value;
          midiRamps[i].need_reset = synth1_ramps[ccSeed].reset_after_use;
          midiRamps[i].value = synth1_ramps[ccSeed].cc_default_value;
          midiRamps[i].stepPer16th = (float)(random(-100, 100)) * 0.05f ;
          if (abs(midiRamps[i].stepPer16th) < 0.5 ) {
            midiRamps[i].stepPer16th = 0.5;
          }
          midiRamps[i].leftBars = random(1, 3) * 2;
        } else if (chanSeed < 86)  {
          do {
            ccSeed = random(0, NUM_SYNTH_CCS);
          } while (ramp_cc_repeated(synth2_ramps[ccSeed].cc_number, SYNTH2_MIDI_CHAN));
          midiRamps[i].chan = SYNTH2_MIDI_CHAN;
          midiRamps[i].cc_number = synth2_ramps[ccSeed].cc_number;
          midiRamps[i].min_val = synth2_ramps[ccSeed].cc_min_value;
          midiRamps[i].max_val = synth2_ramps[ccSeed].cc_max_value;
          midiRamps[i].def_val = synth2_ramps[ccSeed].cc_default_value;
          midiRamps[i].need_reset = synth2_ramps[ccSeed].reset_after_use;
          midiRamps[i].value = synth2_ramps[ccSeed].cc_default_value;
          midiRamps[i].stepPer16th = (float)(random(-100, 100)) * 0.05f ;
          if (abs(midiRamps[i].stepPer16th) < 0.5 ) {
            midiRamps[i].stepPer16th = 0.5;
          }
          midiRamps[i].leftBars = random(1, 3) * 2;
        } else {
          do {
            ccSeed = random(0, NUM_DRUM_CCS);
          } while (ramp_cc_repeated(drum_ramps[ccSeed].cc_number, DRUM_MIDI_CHAN));
          midiRamps[i].chan = DRUM_MIDI_CHAN;
          midiRamps[i].cc_number = drum_ramps[ccSeed].cc_number;
          midiRamps[i].min_val = drum_ramps[ccSeed].cc_min_value;
          midiRamps[i].max_val = drum_ramps[ccSeed].cc_max_value;
          midiRamps[i].def_val = drum_ramps[ccSeed].cc_default_value;
          midiRamps[i].need_reset = drum_ramps[ccSeed].reset_after_use;
          midiRamps[i].value = drum_ramps[ccSeed].cc_default_value;
          midiRamps[i].stepPer16th = (float)(random(-100, 100)) * 0.1f ;
          if (abs(midiRamps[i].stepPer16th) < 0.5 ) {
            midiRamps[i].stepPer16th = 0.5;
          }
          midiRamps[i].leftBars = random(1, 3) ;
        }
      }
    }
  } else { // if Break.status == sPlaying
    if (bar_current == Break.start) {
      for (int i = 0; i < NUM_RAMPS; i++) {
        /*
          if (midiRamps[i].need_reset) {
          send_midi_control(midiRamps[i].chan, midiRamps[i].cc_number, midiRamps[i].def_val);
          }
        */
        midiRamps[i].leftBars = Break.length;
        midiRamps[i].value = midiRamps[i].min_val;
        midiRamps[i].stepPer16th = ((float)(midiRamps[i].max_val - midiRamps[i].min_val) / (float)(Break.length * PatternLength));
      }
    }
  } // if Break.status
}

static void do_midi_tick() {
  send_midi_tick();
  midi_tick++;
  if (midi_tick >= MIDI_TICKS_PER_16TH) {
    midi_tick = 0;
    midi_step++;
    if (midi_step >= PatternLength) {
      bar_current++;
      midi_step = 0;
      decide_on_break();
      check_midi_ramps(false); // every bar
    }
    sequencer_step(midi_step);
  }
}

static void do_midi_stop() {
  instr_allnotesoff();
  send_midi_stop();
  midi_playing = 0;
}

/*
   Instrument definition
*/

static const byte drum_notes[6] = { KICK_NOTE, SNARE_NOTE, CLOSED_HAT_NOTE, OPEN_HAT_NOTE, PERCUSSION_NOTE, CRASH_NOTE };
static const byte synth_midi_channels[2] = { SYNTH1_MIDI_CHAN, SYNTH2_MIDI_CHAN };

static void init_instruments() {
  Instrument *ins = &instruments[0];

  // Make synth instruments
  for (int i = 0; i < 2; i++) {
    ins->midi_channel = synth_midi_channels[i];
    ins->is_drum = 0;
    ins->noteon = send_midi_noteon;
    ins->noteoff = send_midi_noteoff;
    ins++;
  }

  // Make drum instruments
  for (int i = 0; i < 6; i++) {
    ins->midi_channel = DRUM_MIDI_CHAN;
    ins->is_drum = 1;
    ins->drum_note = drum_notes[i];
    ins->noteon = send_midi_noteon;
    ins->noteoff = send_midi_noteoff;
    ins++;
  }
}

/*
   Main program
*/

/*
  void setup() {
  init_midi();
  for (int i = 0; i < ButLast; i++) {
    init_button(&buttons[i], button_pins[i]);
  }
  init_instruments();
  init_patterns();
  }
*/

void start_midi_clock() {
}

void run_ui() {
  int8_t source_memory = -1;

  // If memory button is pressed, then it is a source_memory
  for (int i = 0; i < NumMemories; i++) {
    if (is_pressed(ButMem1 + i) && !just_pressed(ButMem1 + i)) {
      source_memory = i;
    }
  }

  // If a memory button is pressed, then switch to it.
  // But if another memory button is pressed, then copy one to another.
  for (int i = 0; i < NumMemories; i++) {
    if (just_pressed(ButMem1 + i)) {
      if (source_memory >= 0) {
#ifdef DEBUG_JUKEBOX
        DEBF("copy %d to %d\r\n", source_memory, i);
#endif
        memcpy(&memories[i], &memories[source_memory], sizeof(memories[0]));
      } else {
#ifdef DEBUG_JUKEBOX
        DEBF("switching to memory %d", i);
#endif
        cur_memory = i;
      }
    }
  }

  // Handle pattern generation
  if (just_pressed(ButPat1)) {
    mem_generate_melody_and_seed(cur_memory, 0);
    print_memory(cur_memory);
  }
  if (just_pressed(ButPat1 + 1)) {
    mem_generate_melody_and_seed(cur_memory, 1);
    print_memory(cur_memory);
  }
  if (just_pressed(ButNotes)) {
    mem_generate_note_set(cur_memory);
    print_memory(cur_memory);
  }
  if (just_pressed(ButDrums)) {
    mem_generate_drums(cur_memory, DrumStraight);
    print_memory(cur_memory);
  }

  // Handle play
  if (just_pressed(ButPlay)) {
    if (midi_playing) {
#ifdef DEBUG_JUKEBOX
      DEBUG("stopping midi");
#endif
      do_midi_stop();
    } else {
#ifdef DEBUG_JUKEBOX
      DEBF("starting midi clock, dt=%lu", midi_tick_ms);
#endif
      last_midi_tick = now;
      do_midi_start();
      do_midi_tick();
    }
  }
}

void run_tick() {
  /* Run button scan at 250 Hz */
  now = millis();
  button_divider++;
  if (button_divider >= 4) {
    for (int i = 0; i < ButLast; i++) {
      read_button(&buttons[i]);
    }
    run_ui();
    button_divider = 0;
    
  }

  /* If MIDI is playing, then check for tick */
  if (midi_playing && (now - last_midi_tick) >= midi_tick_ms) {
    do_midi_tick();
    last_midi_tick += midi_tick_ms;
    if (last_midi_tick < now) {
      /* we are at least one tick late, give up */
      last_midi_tick = now;
    }
  }
}

/*
  static unsigned long last_ms = 0;

  void loop() {
  // wait until next ms
  do {
    now = millis();
  } while (now == last_ms);
  last_ms = now;
  // advance one tick
  run_tick();
  myRandomAddEntropy(analogRead(0));
  }
*/

#endif
