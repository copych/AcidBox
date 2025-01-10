#pragma once
/*
   FAST Pseudo-random generator
*/

#define XORSHIFT 1
#define LFSR 2
#define CONGRUENT 3

#define RANDOM_ALGO  XORSHIFT
//#define RANDOM_ALGO  LFSR
//#define RANDOM_ALGO  CONGRUENT

const uint32_t WORD_ALIGNED_ATTR MYRAND_MAX = 0xFFFFFFFF;
const uint32_t WORD_ALIGNED_ATTR MYRAND_MAGIC = 0xcf300000;
// const uint32_t MYRAND_MAGIC = 0xBF000000;

class MyRand {

public:
  MyRand(){setSeed();};
  ~MyRand(){};
  
  inline uint32_t getRaw() {
    _myRandomState = next(_myRandomState);
    return _myRandomState;
  }
  
  inline uint32_t getState() {
    return _myRandomState;
  }

  inline void saveState() {
    _mySavedState = _myRandomState;
  }

  inline void loadState() {    
    _myRandomState = _mySavedState;
  }

  inline uint32_t getUnsignedInt(uint32_t max) {
    if (max==0) return 0;
    return getRaw() % max;
  }

  inline float getFloat() {
    return (float)(getRaw()) * TO_FLOAT;
  }

  inline float getFloat(float& upper_limit) {
    return ( upper_limit * TO_FLOAT * (float)(getRaw() ) ) ;
  }
  
  inline float getFloatInRange(const float& lower_limit, const float& upper_limit) {
    return (float)lower_limit + ( (float)(upper_limit - lower_limit) * (float)TO_FLOAT * (float)(getRaw() ) ) ;
  }

  inline float getFloatSpread(const float& center_val, const float& variation) {
    return center_val - variation + ( variation * TO_FLOAT_2 * (float)(getRaw() ) ) ;
  }
  
  inline void randomize(uint32_t data) {
    _myRandomState = next((_myRandomState << 1) ^ data);
  }

  inline void setSeed() {
    randomize((uint32_t)(micros() & MYRAND_MAX));
  }
  
  inline void setSeed(uint32_t seed) {
    _myRandomState = seed;
  }
  
  inline bool tryChance(float chance_normalized) { // e.g. tryChance(0.3f) should return true ~30% times tried
    bool ret (getFloat() < chance_normalized);
    return ret;
  }

  inline void init() {
    randomize(random(3, MYRAND_MAX));
  }
  

private:  
  const float WORD_ALIGNED_ATTR TO_FLOAT = 1.0f / MYRAND_MAX;
  const float WORD_ALIGNED_ATTR TO_FLOAT_2 = 2.0f / MYRAND_MAX;
  uint32_t WORD_ALIGNED_ATTR _myRandomState = 1664525UL ;
  uint32_t WORD_ALIGNED_ATTR _mySavedState = _myRandomState ;
  const uint32_t WORD_ALIGNED_ATTR _a = 1664525UL ;
  const uint32_t WORD_ALIGNED_ATTR _c = 1013904223UL;

#if RANDOM_ALGO == LFSR
// lfsr32
  inline uint32_t next(uint32_t x) {
    uint32_t y ;
    y = x >> 1;
    if (x & 1) y ^= MYRAND_MAGIC;
    return y;
  }
#elif RANDOM_ALGO == XORSHIFT
// xorshift32

  inline uint32_t next(uint32_t x)
  {
      x ^= x << 13;
      x ^= x >> 17;
      x ^= x << 5;
      return x;
  }


/*
unsigned long next(unsigned long x) {
    unsigned long result;

    asm volatile (
        "slli %[y], %[x], 13\n\t"    // y = x << 13
        "xor %[x], %[x], %[y]\n\t"   // x ^= y
        "extui %[y], %[x], 17, 15\n\t" // y = (x >> 17) (extract unsigned immediate)
        "xor %[y], %[y], %[x]\n\t"    // y ^= x
        "slli %[x], %[y], 5\n\t"      // x = y << 5
        "xor %[x], %[x], %[y]\n\t"    // x ^= y
        : [y] "=r" (result)           // output operand: result   
        : [x] "r" (x)               // input/output operand: x     
        :                             // no clobbered registers
    );

    return result;
}
*/


#elif RANDOM_ALGO == CONGRUENT
// Linear Congruential Generator
  inline uint32_t next(uint32_t x)
  {
      uint32_t y;
      y = (uint32_t)_a * (uint32_t)x + (uint32_t)_c;      
      return y;
  }
#endif

};
