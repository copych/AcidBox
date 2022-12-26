
inline void handleNoteOn(uint8_t inChannel, uint8_t inNote, uint8_t inVelocity) {

  if (inChannel == SYNTH1_MIDI_CHAN ) {Synth1.StartNote(inNote, inVelocity);}
  if (inChannel == SYNTH2_MIDI_CHAN ) {Synth2.StartNote(inNote, inVelocity);}
  if (inChannel == DRUM_MIDI_CHAN ) {Drums.NoteOn(inNote, inVelocity);}
}

inline void handleNoteOff(uint8_t inChannel, uint8_t inNote, uint8_t inVelocity) {

  if (inChannel == SYNTH1_MIDI_CHAN ) {Synth1.EndNote(inNote, inVelocity);}
  if (inChannel == SYNTH2_MIDI_CHAN ) {Synth2.EndNote(inNote, inVelocity);}
  if (inChannel == DRUM_MIDI_CHAN ) {Drums.NoteOff(inNote);}

}

inline void handleCC(uint8_t inChannel, uint8_t cc_number, uint8_t cc_value) {
  switch (cc_number) { // global parameters yet set via ANY channel CCs
    case CC_ANY_COMPRESSOR:
      Comp.SetRatio(3.0f + cc_value * 0.307081f);
      break;
#ifndef NO_PSRAM
    case CC_ANY_DELAY_TIME:
      Delay.SetLength(cc_value * MIDI_NORM);
      break;
    case CC_ANY_DELAY_FB:
      Delay.SetFeedback(cc_value * MIDI_NORM);
      break;
    case CC_ANY_DELAY_LVL:
      Delay.SetLevel(cc_value * MIDI_NORM);
      break;
    case CC_ANY_REVERB_TIME:
      Reverb.SetTime(cc_value * MIDI_NORM);
      break;
    case CC_ANY_REVERB_LVL:
      Reverb.SetLevel(cc_value * MIDI_NORM);
      break;
#endif
    default:
      if (inChannel == SYNTH1_MIDI_CHAN ) {Synth1.ParseCC(cc_number, cc_value);}
      if (inChannel == SYNTH2_MIDI_CHAN ) {Synth2.ParseCC(cc_number, cc_value);}
      if (inChannel == DRUM_MIDI_CHAN ) {Drums.ParseCC(cc_number, cc_value);}
  }
}

void handleProgramChange(uint8_t channel, uint8_t number) {
  if (channel == 10) {
    Drums.SetProgram(number);
  }
}
