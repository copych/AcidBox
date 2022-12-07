#ifndef MIDI_CONTROLS_H
#define MIDI_CONTROLS_H

// 303 Synths MIDI CC
#define CC_303_PORTATIME  5
#define CC_303_VOLUME     7
#define CC_303_PORTAMENTO 65
#define CC_303_PAN        10
#define CC_303_WAVEFORM   70
#define CC_303_RESO       71
#define CC_303_CUTOFF     74
#define CC_303_ATTACK     73
#define CC_303_DECAY      72
#define CC_303_ENVMOD_LVL 75
#define CC_303_ACCENT_LVL 76
#define CC_303_REVERBSEND 91
#define CC_303_DELAYSEND  92
#define CC_303_DISTORTION 94

// 808 Drums MIDI CC
#define CC_808_VOLUME     7
#define CC_808_PAN        10
#define CC_808_RESO       71
#define CC_808_CUTOFF     74
#define CC_808_REVERBSEND 91
#define CC_808_DELAYSEND  92
#define CC_808_DISTORTION 94
#define CC_808_PITCH      89
#define CC_808_NOTE_SEL   90  // select note, all the following CC modifiers will be applied to this sample
#define CC_808_ATTACK     73
#define CC_808_DECAY      72

// Global effects (I know they should be managed by SysEx, but to keep it more simple, I bind them to the unoccupied CC messages on any channel)
#define CC_ANY_COMPRESSOR 93
#define CC_ANY_DELAY_TIME 84
#define CC_ANY_DELAY_FB   85
#define CC_ANY_DELAY_LVL  86
#define CC_ANY_REVERB_TIME 87
#define CC_ANY_REVERB_LVL 88


#endif
