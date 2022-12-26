#ifdef JUKEBOX

//  _______ _     _ _______      _______ __   _ ______         _______ _______ _______
//     |    |_____| |______      |______ | \  | |     \ |      |______ |______ |______
//     |    |     | |______      |______ |  \_| |_____/ |_____ |______ ______| ______|
//
//  _______ _______ _____ ______       ______  _______ __   _  ______ _______  ______
//  |_____| |         |   |     \      |_____] |_____| | \  | |  ____ |______ |_____/
//  |     | |_____  __|__ |_____/      |_____] |     | |  \_| |_____| |______ |    \_
//
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


#define KICK_NOTE               0 //001
#define SNARE_NOTE              1 //002
#define CLOSED_HAT_NOTE         6 //007
#define OPEN_HAT_NOTE           7 //008
#define PERCUSSION_NOTE         4 //005

// Pin numbers to which are buttons attached (connect one side of button to pin, the other to ground)
#define GEN_SYNTH1_BUTTON_PIN   0
#define GEN_SYNTH2_BUTTON_PIN   23
#define GEN_NOTES_BUTTON        23
#define GEN_DRUM_BUTTON         23
#define PLAY_BUTTON             23
#define MEM1_BUTTON             23
#define MEM2_BUTTON             23
#define MEM3_BUTTON             23
#define MEM4_BUTTON             23
#define MEM5_BUTTON             23

// The BPM setting is very coarse, because it's based on `millis` clock and the
// time between two midi ticks (1/6th of 16th note) has to be an integral number
// of milliseconds...
#define BPM 130
#define NUM_RAMPS 6 // simultaneous knob rotatings
#ifndef NO_PSRAM
  #define NUM_SYNTH_CCS 11
  #define NUM_DRUM_CCS 6
#else
  #define NUM_SYNTH_CCS 9
  #define NUM_DRUM_CCS 4
#endif

struct sSynthCCs {
  uint8_t cc_number;
  uint8_t cc_couple;
  uint8_t cc_default_value;
  uint8_t cc_min_value;
  uint8_t cc_max_value;
  bool reset_after_use;  
};

sSynthCCs synth1_ramps[NUM_SYNTH_CCS] = {
 //cc   cpl def   min max   reset
  {71,  74, 64,   0,  110,  true},
  {74,  71, 20,   0,  70,   true},
  {10,  0,  10,   0,  127,  true},
  {75,  0,  100,  0,  127,  false},
  {70,  0,  127,  0,  127,  true},
#ifndef NO_PSRAM
  {91,  0,  5,    2,  127,  true},
  {92,  0,  0,    64, 127,  false},
#endif
  {94,  0,  10,   2,  120,  true},
  {95,  0,  25,   25, 100,  false},
  {72,  0,  20,   10, 80,   true},
  {73,  0,  1,    3,  20,   true}
};

sSynthCCs synth2_ramps[NUM_SYNTH_CCS] = {
 //cc   cpl def   min max   reset
  {71,  74, 64,   0,  122,  true},
  {74,  71, 20,   0,  70,   true},
  {10,  0,  117,  0,  127,  true},
  {75,  0,  64,   0,  127,  false},
  {70,  0,  0,    0,  127,  true},
  {94,  0,  10,   2,  120,  true},
  {95,  0,  25,   25, 100,  false},
#ifndef NO_PSRAM
  {91,  0,  3,    2,  127,  true},
  {92,  0,  0,    60, 127,  false},
#endif
  {72,  0,  20,   10, 80,   true},
  {73,  0,  0,    3,  20,   true}
};

sSynthCCs drum_ramps[NUM_DRUM_CCS] = {
 //cc   cpl def   min max   reset
  {74,  0,  64,   0,  127,  true},
  {71,  0,  0,    0,  127,  true},
#ifndef NO_PSRAM
  {91,  0,  5,    2,  127,  true},
  {92,  0,  0,    64, 127,  false},
#endif
  {93,  0,  15,   80, 127,  true},
  {94,  0,  6,    6,  100,  true}
};

#define send_midi_start() {} 
#define send_midi_stop()  {}
#define send_midi_tick() {}

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

struct Button {
  uint8_t history;
  byte pin;
  uint8_t numb;
};

enum {
  NumInstruments = 2 + 5,
  AccentedMidiVol = 110,
  NormalMidiVol = 70,
};

typedef struct Instrument Instrument;

struct Instrument {
  // 1-base indexed MIDI channel (first channel is 1)
  byte midi_channel;
  byte is_drum, drum_note;
  void (*noteon)(byte chan, byte note, byte vel);
  void (*noteoff)(byte chan, byte note);
  byte playing_note;
};

static Instrument instruments[NumInstruments];


enum {
  NumMemories = 5,
  MaxNoteSet = 16,
  PatternLength = 16,
};

typedef struct Pattern Pattern;
typedef struct Memory Memory;

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

enum {
  ButPat1 = 0,
  ButNotes = 2,
  ButDrums = 3,
  ButPlay = 4,
  ButMem1 = 5,
  ButLast = ButMem1 + NumMemories,
};

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
  // MIDI.sendNoteOn(note, vol, chan);
  handleNoteOn( chan, note, vol) ;
#ifdef DEBUG_JUKEBOX
  DEBUG("note on");
#endif
}
static void send_midi_noteoff(byte chan, byte note) {
  //MIDI.sendNoteOn(note, 0, chan);
  handleNoteOff( chan, note, 0) ;
}
static void init_midi() {
//  Serial.begin(115200);
//  MIDI.begin(MIDI_CHANNEL_OMNI);
  
  for (byte i = 0; i < ButLast; i++) {
    init_button(&buttons[i], button_pins[i], i+1 );
  }
  init_instruments();
  init_patterns(); 
  do_midi_start();
}
static void send_midi_control(byte chan, byte cc_number, byte cc_value) {
  //MIDI.sendControlChange (  cc_number,  cc_value,  chan);   
  handleCC( chan,  cc_number,  cc_value);
}

/*
 * Pseudo-random generator with restorable state
 */

static inline uint16_t lfsr16_next(uint16_t x)
{
	uint16_t y = x >> 1;
	if (x & 1)
		y ^= 0xb400;
	return y;
}

//static uint16_t myRandomState = 0x1234;
static uint16_t myRandomState = (uint16_t)(random(0,0xffff));

static uint16_t myRandomAddEntropy(uint16_t data) {
  myRandomState = lfsr16_next((myRandomState << 1) ^ data);
  return myRandomState;
}

static uint16_t myRandomRaw() {
  myRandomState = lfsr16_next(myRandomState);
  return myRandomState;
}

static inline uint16_t myRandom(uint16_t max) {
  return myRandomRaw() % max;
}

/*
 * Buttons
 */

static void read_button(struct Button *button)
{
  if (button->numb==1) {
    button->history = (button->history << 1) | (digitalRead(button->pin) == HIGH);
  } else if (button->numb<1 || button->numb>1) {
    uint32_t btnSeed = random(1000000);
    if (btnSeed<10 && button->numb!=5) {
      button->history = 0x80;
    } else {
      button->history = 0;
    }
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
 * Instruments
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
  for (byte i = 0; i < NumInstruments; i++) {
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
#ifdef DEBUG_JUKEBOX
  DEBF("glide=%d accent=%d\r\n", do_glide, do_accent);
#endif
  if (ins->is_drum) {
    // For drums: value is volume, accent and glide are ignored
    instr_noteon_raw(instr, ins->drum_note, value, 0);
  } else {
    // For drums: value is note, volume is accent, glide is used
    instr_noteon_raw(instr, value, do_accent ? AccentedMidiVol : NormalMidiVol, do_glide);
  }
}

/*
 * Sequencer
 */

void sequencer_step(byte step) {
#ifdef DEBUG_JUKEBOX
  DEBF("midi step %d\r\n", step);
#endif
  do_midi_ramps();
  // Play all notes in current step
  for (byte i = 0; i < NumInstruments; i++) {
    Pattern *pat = &memories[cur_memory].patterns[i];
    byte accent = (pat->accent >> step) & 1;
    byte glide = (pat->glide >> step) & 1;
    byte value = pat->notes[step];

    if (value > 0)
      instr_noteon(i, value, glide, accent);
    else
      instr_noteoff(i);
  }
}

/*
 * Endless Acid Banger pattern generator
 * 
 * Adapted from https://www.vitling.xyz/toys/acid-banger/
 * created by Vitling (David Whiting) i.am@thewit.ch
 */

#define NOTE_LIST(x...) (int8_t[]) { x, -1 }
//#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

static const int8_t *const offset_choices[] = {
  NOTE_LIST(0, 0, 12, 24, 27),
  NOTE_LIST(0, 0, 0, 12, 10, 19, 26, 27),
  NOTE_LIST(0, 1, 7, 10, 12, 13),
  NOTE_LIST(0),
  NOTE_LIST(0, 0, 0, 12),
  NOTE_LIST(0, 0, 12, 14, 15, 19),
  NOTE_LIST(0, 0, 0, 0, 12, 13, 16, 19, 22, 24, 25),
  NOTE_LIST(0, 0, 0, 7, 12, 15, 17, 20, 24),
};

static byte generate_note_set(uint8_t *note_set) {
  // Random root note
  byte root = myRandom(15) + 28;

  // Random note set
  byte set = myRandom(ARRAY_SIZE(offset_choices));

  // Copy notes from note set and offset them by "root note"
  byte i;
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
  for (byte i = 0; i < pattern_len; i++) {
    uint8_t chance = ((uint16_t) density * (i % 4 == 0 ?  60 : (i % 3 == 0 ?  50 : (i % 2 == 0 ?  30 : 10)))) >> 8;
    if (flip(chance)) {
      pattern[i] = note_set[myRandom(note_set_len)];
      if (flip(30))
        *accent |= 1u << i;
      if (flip(10))
        *glide |= 1u << i;
    } else {
      pattern[i] = 0;
    }
  }
}

enum {
  KickElectro,
  KickFourFloor,
};

enum {
  SnareBackbeat,
  SnareSkip,
  SnareNone,
};

enum {
  HatsOffbeats,
  HatsClosed,
  HatsNone,
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

static void generate_drums(byte *kick, byte *snare, byte *oh, byte *ch, byte *perc) {
  byte kick_mode = myRandom(2);
  byte hat_mode = myRandom(SnareNone);
  byte snare_mode = myRandom(HatsNone);
  byte perc_mode = myRandom(PercNone);

  memset(kick, 0, PatternLength);
  memset(snare, 0, PatternLength);
  memset(oh, 0, PatternLength);
  memset(ch, 0, PatternLength);
  memset(perc, 0, PatternLength);

  if (kick_mode == KickFourFloor) {
    for (byte i = 0; i < PatternLength; i++) {
      if (i % 4 == 0)
        kick[i] = 120;
      else if (i % 2 == 0 && flip(10))
        kick[i] = 80;
    }
  } else if (kick_mode == KickElectro) {
    for (byte i = 0; i < PatternLength; i++) {
      if (i == 0)
        kick[i] = 127;
      else if (i % 2 == 0 && i % 8 != 4 && flip(50))
        kick[i] = myRandom(110);
      else if (flip(5))
        kick[i] = myRandom(110);
    }
  }

  if (snare_mode == SnareBackbeat) {
    for (byte i = 0; i < PatternLength; i++) {
      if (i % 8 == 0)
        snare[i] = 127;
    }
  } else if (snare_mode == SnareSkip) {
    for (byte i = 0; i < PatternLength; i++) {
      if (i % 8 == 3 || i % 8 == 6)
        snare[i] = 90 + myRandom(37);
      else if (i % 2 == 0 && flip(20))
        snare[i] = 40 + myRandom(25);
      else if (flip(10))
        snare[i] = 25 + myRandom(25);
    }
  }

  if (hat_mode == HatsOffbeats) {
    for (byte i = 0; i < PatternLength; i++) {
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
    for (byte i = 0; i < PatternLength; i++) {
      if (i % 2 == 0)
        ch[i] = 50;
      else if (flip(50))
        ch[i] = myRandom(40);
    }
  }

  if (perc_mode == PercFiller) {
    for (byte i = 0; i < PatternLength; i++) {
      if (oh[i] == 0 && ch[i] == 0 && kick[i] == 0 && snare[i] == 0)
        perc[i] = 90 + myRandom(37);
    }
  } else if (perc_mode == PercXor1) {
    for (byte i = 0; i < PatternLength; i++) {
      if ((kick[i] == 0) ^ (snare[i] == 0))
        perc[i] = 90 + myRandom(37);
    }
  } else if (perc_mode == PercXor2) {
    for (byte i = 0; i < PatternLength; i++) {
      if ((oh[i] == 0) ^ (ch[i] == 0))
        perc[i] = 90 + myRandom(37);
    }
  } else if (perc_mode == PercEcho) {
    byte distance = 1 + myRandom(7);

    for (byte i = 0; i < PatternLength; i++) {
      byte prev_step = (i + PatternLength - distance) % PatternLength;
      if (ch[prev_step] || oh[prev_step])
        perc[i] = 90 + myRandom(37);
    }
  } else if (perc_mode == PercRolls) {
    byte roll = 0, roll_vol = 0;
    for (byte i = 0; i < PatternLength; i++) {
      byte do_roll = 0;
      if (i % 8 == 3 && flip(40))
        do_roll = 1;
      else if (i % 2 == 0 && flip(20))
        do_roll = 1;
      else if (flip(10))
        do_roll = 1;

      if (do_roll) {
        roll_vol = 90 + myRandom(37);
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
 * Generator-to-pattern binding
 */

void mem_generate_melody(byte mem, byte voice) {
  Memory *m = &memories[mem];
  Pattern *p = &m->patterns[voice];
  // Temporarily change random seed to a pre-defined state so that we can generate
  // identical melody.
  uint16_t random_state = myRandomState;
  myRandomState = (m->random_seed << 1) ^ voice;
#ifdef DEBUG_JUKEBOX
  DEBF("generating %d/%d with seed %u", mem, voice, myRandomState);
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
  for (byte i = 0; i < 2; i++)
    mem_generate_melody(mem, i);
}

void mem_generate_drums(byte mem) {
  Memory *m = &memories[mem];
  generate_drums(
    m->patterns[2].notes,
    m->patterns[3].notes,
    m->patterns[5].notes,
    m->patterns[4].notes,
    m->patterns[6].notes);
}

void mem_generate_all(byte mem) {
  mem_generate_note_set(mem);
  mem_generate_drums(mem);
  for (byte i = 0; i < 2; i++)
    mem_generate_melody_and_seed(mem, i);
}

void print_pattern(struct Pattern *p, byte is_drum) {
#ifdef DEBUG_JUKEBOX
  for (byte i = 0; i < PatternLength; i++)
  DEBF("%3d \r\n", p->notes[i]);

  if (!is_drum) {
    for (byte i = 0; i < PatternLength; i++)
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
  for (byte i = 0; i < m->num_notes_in_set; i++)
#ifdef DEBUG_JUKEBOX
    DEBF(" %d\r\n", m->note_set[i]);
#endif
  for (byte i = 0; i < NumInstruments; i++)
    print_pattern(&m->patterns[i], instruments[i].is_drum);
}

void init_patterns() {
  for (byte i = 0; i < NumMemories; i++)
    mem_generate_all(i);
}

/*
 * MIDI clock
 */

#define MIDI_TICKS_PER_16TH 6

static const unsigned long midi_tick_ms = 1000ul * 15 / BPM / MIDI_TICKS_PER_16TH;
static byte midi_playing, midi_tick, midi_step;

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
  send_midi_control(SYNTH1_MIDI_CHAN, 7, 80);
  send_midi_control(SYNTH2_MIDI_CHAN, 7, 80);
  send_midi_control(DRUM_MIDI_CHAN,   7, 127);
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
    }
    if (midiRamps[i].value <= midiRamps[i].min_val ) {
      midiRamps[i].value = midiRamps[i].min_val;
      midiRamps[i].stepPer16th = -midiRamps[i].stepPer16th;
    }
    uint8_t val = (uint8_t)(midiRamps[i].value);
    send_midi_control(midiRamps[i].chan,  midiRamps[i].cc_number, val);
#ifdef DEBUG_JUKEBOX
    DEB("ramp: ");
    DEB(midiRamps[i].chan);
    DEB(" ");
    DEB(midiRamps[i].cc_number);
    DEB(" ");
    DEBUG(val);
#endif
  }
}

static void check_midi_ramps() {
  // TODO escape doubling CCs
  for (int i = 0; i < NUM_RAMPS; i++) {
    midiRamps[i].leftBars--;
    if (midiRamps[i].leftBars <= 0) { // no more bars left for the ramp
      if (midiRamps[i].need_reset) {
        send_midi_control(midiRamps[i].chan, midiRamps[i].cc_number, midiRamps[i].def_val);
      }
      uint8_t chanSeed = random(0,100); // probability
      uint8_t ccSeed;
/*
  uint8_t chan = 0;
  uint8_t cc_number = 0;
  float value = 0.0f;
  float min_val = 0.0f;
  float max_val = 127.0f;
  float def_val = 64.0f;
  bool need_reset = false;
  float stepPer16th = 0.0f;
  int16_t leftBars = 0;

  uint8_t cc_number;
  uint8_t cc_couple;
  uint8_t cc_default_value;
  uint8_t cc_min_value;
  uint8_t cc_max_value;
  bool reset_after_use;  
*/
      if (chanSeed<50) {
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
        midiRamps[i].stepPer16th = (float)(random(-100,100))*0.05f ;
        if (abs(midiRamps[i].stepPer16th) < 0.5 ) {midiRamps[i].stepPer16th = 0.5;}
        midiRamps[i].leftBars = random(1,3)*2;
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
        midiRamps[i].stepPer16th = (float)(random(-100,100))*0.05f ;
        if (abs(midiRamps[i].stepPer16th) < 0.5 ) {midiRamps[i].stepPer16th = 0.5;}
        midiRamps[i].leftBars = random(1,3)*2; 
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
        midiRamps[i].stepPer16th = (float)(random(-100,100))*0.1f ;
        if (abs(midiRamps[i].stepPer16th) < 0.5 ) {midiRamps[i].stepPer16th = 0.5;}
        midiRamps[i].leftBars = random(1,3) ;
      }
    }
  }
}

static void do_midi_tick() {
  send_midi_tick();
  midi_tick++;
  if (midi_tick >= MIDI_TICKS_PER_16TH) {
    midi_tick = 0;
    midi_step++;
    if (midi_step >= PatternLength) {
      midi_step = 0;
      check_midi_ramps(); // every bar      
    }
    sequencer_step(midi_step);
    do_midi_ramps();
  }
}

static void do_midi_stop() {
  instr_allnotesoff();
  send_midi_stop();
  midi_playing = 0;
}

/*
 * Instrument definition
 */

static const byte drum_notes[5] = { KICK_NOTE, SNARE_NOTE, CLOSED_HAT_NOTE, OPEN_HAT_NOTE, PERCUSSION_NOTE };
static const byte synth_midi_channels[2] = { SYNTH1_MIDI_CHAN, SYNTH2_MIDI_CHAN };

static void init_instruments() {
  Instrument *ins = &instruments[0];

  // Make synth instruments
  for (byte i = 0; i < 2; i++) {
    ins->midi_channel = synth_midi_channels[i];
    ins->is_drum = 0;
    ins->noteon = send_midi_noteon;
    ins->noteoff = send_midi_noteoff;
    ins++;
  }

  // Make drum instruments
  for (byte i = 0; i < 5; i++) {
    ins->midi_channel = DRUM_MIDI_CHAN;
    ins->is_drum = 1;
    ins->drum_note = drum_notes[i];
    ins->noteon = send_midi_noteon;
    ins->noteoff = send_midi_noteoff;
    ins++;
  }
}

/*
 * Main program
 */

/*
void setup() {
  init_midi();
  for (byte i = 0; i < ButLast; i++) {
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
  for (byte i = 0; i < NumMemories; i++) {
    if (is_pressed(ButMem1 + i) && !just_pressed(ButMem1 + i)) {
      source_memory = i;
    }
  }

  // If a memory button is pressed, then switch to it.
  // But if another memory button is pressed, then copy one to another.
  for (byte i = 0; i < NumMemories; i++) {
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
    mem_generate_drums(cur_memory);
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
    for (byte i = 0; i < ButLast; i++) {
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
