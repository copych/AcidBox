#ifndef MIDI_CONTROLS_H
#define MIDI_CONTROLS_H

#define GM_MIDI
//#define VINTAGE_MIDI

#ifdef GM_MIDI
// 303 Synths MIDI CC
#define CC_303_PORTATIME    5
#define CC_303_VOLUME       7
#define CC_303_PORTAMENTO   65
#define CC_303_PAN          10
#define CC_303_WAVEFORM     70
#define CC_303_RESO         71
#define CC_303_CUTOFF       74
#define CC_303_ATTACK       73
#define CC_303_DECAY        72
#define CC_303_ENVMOD_LVL   75
#define CC_303_ACCENT_LVL   76
#define CC_303_REVERB_SEND  91
#define CC_303_DELAY_SEND   92
#define CC_303_DISTORTION   94
#define CC_303_OVERDRIVE    95
#define CC_303_SATURATOR    128
#define CC_303_TUNING       104

// 808 Drums MIDI CC
#define CC_808_VOLUME       7
#define CC_808_NOTE_PAN     8
#define CC_808_PAN          10
#define CC_808_RESO         71
#define CC_808_CUTOFF       74
#define CC_808_REVERB_SEND  91
#define CC_808_DELAY_SEND   92
#define CC_808_DISTORTION   94
#define CC_808_PITCH        89
#define CC_808_NOTE_SEL     90  // select note, all the following CC note modifiers will be applied to this sample as it was RPN or NRPN
#define CC_808_NOTE_ATTACK  73
#define CC_808_NOTE_DECAY   72
#define CC_808_BD_TONE      21  // Specific per drum control
#define CC_808_BD_DECAY     23
#define CC_808_BD_LEVEL     24
#define CC_808_SD_TONE      25
#define CC_808_SD_SNAP      26
#define CC_808_SD_LEVEL     29
#define CC_808_CH_TUNE      61
#define CC_808_CH_LEVEL     63
#define CC_808_OH_TUNE      80
#define CC_808_OH_DECAY     81
#define CC_808_OH_LEVEL     82

// Global 
#define CC_ANY_COMPRESSOR   93
#define CC_ANY_DELAY_TIME   84
#define CC_ANY_DELAY_FB     85
#define CC_ANY_DELAY_LVL    86
#define CC_ANY_REVERB_TIME  87
#define CC_ANY_REVERB_LVL   88
#define CC_ANY_RESET_CCS    121
#define CC_ANY_NOTES_OFF    123
#define CC_ANY_SOUND_OFF    120
#endif




#ifdef VINTAGE_MIDI
// 303 Synths MIDI CC
#define CC_303_PORTATIME    5
#define CC_303_VOLUME       11
#define CC_303_PORTAMENTO   65
#define CC_303_PAN          10
#define CC_303_WAVEFORM     70
#define CC_303_RESO         71
#define CC_303_CUTOFF       74
#define CC_303_ATTACK       73
#define CC_303_DECAY        75
#define CC_303_ENVMOD_LVL   12
#define CC_303_ACCENT_LVL   76
#define CC_303_REVERB_SEND  91
#define CC_303_DELAY_SEND   19
#define CC_303_DISTORTION   17
#define CC_303_SATURATOR    95
#define CC_303_TUNING       104

// 808 Drums MIDI CC
#define CC_808_VOLUME       7
#define CC_808_NOTE_PAN     8
#define CC_808_PAN          10
#define CC_808_RESO         71
#define CC_808_CUTOFF       74
#define CC_808_REVERB_SEND  91
#define CC_808_DELAY_SEND   92
#define CC_808_DISTORTION   94
#define CC_808_PITCH        89
#define CC_808_NOTE_SEL     90  // select note, all the following CC note modifiers will be applied to this sample as it was RPN or NRPN
#define CC_808_NOTE_ATTACK  73 // universal for all drums
#define CC_808_NOTE_DECAY   72
#define CC_808_BD_TONE      21  // Specific per drum control
#define CC_808_BD_DECAY     23
#define CC_808_BD_LEVEL     24
#define CC_808_SD_TONE      25
#define CC_808_SD_SNAP      26
#define CC_808_SD_LEVEL     29
#define CC_808_CH_TUNE      61
#define CC_808_CH_LEVEL     63
#define CC_808_OH_TUNE      80
#define CC_808_OH_DECAY     81
#define CC_808_OH_LEVEL     82


#define CC_ANY_COMPRESSOR   93
#define CC_ANY_DELAY_TIME   84
#define CC_ANY_DELAY_FB     85
#define CC_ANY_DELAY_LVL    86
#define CC_ANY_REVERB_TIME  87
#define CC_ANY_REVERB_LVL   88
#define CC_ANY_RESET_CCS    121
#define CC_ANY_NOTES_OFF    123
#define CC_ANY_SOUND_OFF    120



#endif
/*
 * TODO: rearrange samples according to 
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

/*
    midiControllerComponent.set303Control("filter", 74);
    midiControllerComponent.set303Control("reso.", 71);
    midiControllerComponent.set303Control("env mod", 12);
    midiControllerComponent.set303Control("decay", 75);
    midiControllerComponent.set303Control("accent", 16);
    midiControllerComponent.set303Control("vcf bd.", 1);
    midiControllerComponent.set303Control("drive", 17);
    midiControllerComponent.set303Control("volume", 11);

    midiControllerComponent.set303Control("delay t.", 18);
    midiControllerComponent.set303Control("delay l.", 19);
    midiControllerComponent.set303Control("tuning", 104);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
  }
  if (TRACK_NUMBER_808 > 0) {
    // max 16 controls!
    midiControllerComponent.set808Control("bd decay", 23);
    midiControllerComponent.set808Control("bd tone", 21);
    midiControllerComponent.set808Control("sd snap", 26);
    midiControllerComponent.set808Control("sd tone", 25);
    midiControllerComponent.set808Control("lt tune", 46);
    midiControllerComponent.set808Control("mt tune", 49);
    midiControllerComponent.set808Control("ht tune", 52);
    midiControllerComponent.set808Control("rs tune", 55);

    midiControllerComponent.set808Control("bd level", 24);
    midiControllerComponent.set808Control("sd level", 29);
    midiControllerComponent.set808Control("ch level", 63);
    midiControllerComponent.set808Control("oh level", 82);
    midiControllerComponent.set808Control("lt level", 48);
    midiControllerComponent.set808Control("mt level", 51);
    midiControllerComponent.set808Control("ht level", 54);
    midiControllerComponent.set808Control("rs level", 57);
*/



/*
tr808
CC# Parameter
9 SHUFFLE
20 BD TUNE
21 BD TONE
22 BD COMP
23 BD DECAY
24 BD LEVEL
25 SD TONE
26 SD SNAPPY
27 SD COMP
28 SD DECAY
29 SD LEVEL
46 LT TUNE
47 LT DECAY
48 LT LEVEL
49 MT TUNE
50 MT DECAY
51 MT LEVEL
52 HT TUNE
53 HT DECAY
54 HT LEVEL
55 RS TUNE
56 RS DECAY
57 RS LEVEL
58 CP TUNE
59 CP DECAY
60 CP LEVEL
61 CH TUNE
62 CH DECAY
63 CH LEVEL
71 TOTAL ACCENT
80 OH TUNE
81 OH DECAY
82 OH LEVEL
83 CY TONE
84 CY DECAY
85 CY LEVEL
86 CB TUNE
87 CB DECAY
88 CB LEVEL

 
 */


 /*
 Note number Sound/Function
24–31   Variation Select A–H
35, 36  Bass Drum
37      Rim shot/Claves
38, 40  Snare Drum
39      HandClap/Maracas
42, 44  Closed HiHat
41, 43  LoTom/LoConga
46      Open HiHat
45, 47 MidTom/MidConga
49 Cymbal
48, 50 HiTom/HiConga
51 Cowbell
 
 */
#endif
