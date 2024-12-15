#ifndef TABLES_H
#define TABLES_H
#include <config.h>

struct Tables {
  static inline float midi_tbl_steps[128];
  static inline float exp_square_tbl[TABLE_SIZE+1];
  static inline float saw_tbl[TABLE_SIZE+1];
  static inline float exp_tbl[TABLE_SIZE+1];
  static inline float knob_tbl[TABLE_SIZE+1]; // exp-like curve
  static inline float shaper_tbl[TABLE_SIZE+1]; // illinear tanh()-like curve
  static inline float lim_tbl[TABLE_SIZE+1]; // diode soft clipping at about 1.0
  static inline float sin_tbl[TABLE_SIZE+1];
  static inline float norm1_tbl[16][16]; // cutoff-reso pair gain compensation
  static inline float norm2_tbl[16][16]; // wavefolder-overdrive gain compensation
};

#endif