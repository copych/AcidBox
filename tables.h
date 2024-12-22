#ifndef TABLES_H
#define TABLES_H
#include <config.h>
class Tables {
  public:
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  midi_tbl_steps[128];
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  exp_square_tbl[TABLE_SIZE+1];
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  saw_tbl[TABLE_SIZE+1];
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  exp_tbl[TABLE_SIZE+1];
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  knob_tbl[TABLE_SIZE+1]; // exp-like curve
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  shaper_tbl[TABLE_SIZE+1]; // illinear tanh()-like curve
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  lim_tbl[TABLE_SIZE+1]; // diode soft clipping at about 1.0
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  sin_tbl[TABLE_SIZE+1];
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  norm1_tbl[16][16]; // cutoff-reso pair gain compensation
  static inline float DRAM_ATTR WORD_ALIGNED_ATTR  norm2_tbl[16][16]; // wavefolder-overdrive gain compensation

  static float bilinearLookup(float (&table)[16][16], float x, float y) __attribute__((noinline));
  static float lookupTable(float (&table)[TABLE_SIZE+1], float index ) __attribute__((noinline));
};
#endif