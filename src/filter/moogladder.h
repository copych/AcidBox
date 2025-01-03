#pragma once
#ifndef DSY_MOOGLADDER_H
#define DSY_MOOGLADDER_H
#include "tables.h"

/** Moog ladder filter module

Ported from soundpipe

Original author(s) : Victor Lazzarini, John ffitch (fast tanh), Bob Moog

General::fast_shape() is now a hybrid linear/table-lookup function to speed up calculations (by copych)
*/
class MoogLadder
{
  public:
    MoogLadder() {}
    ~MoogLadder() {}
    /** Initializes the MoogLadder module.
        sample_rate - The sample rate of the audio engine being run. 
    */
    void Init(float sample_rate);


    /** Processes the lowpass filter
    */
    float Process(float in);

    /** 
        Sets the cutoff frequency or half-way point of the filter.
        Arguments
        - freq - frequency value in Hz. Range: Any positive value.
    */
    inline void SetCutoff(float freq) { freq_ = freq; }

    inline void SetDrive(float drive) { 
      drive_ = drive+0.01f;
      compens_ = (drive_* 0.85f + 3.2f) / drive_;
#ifdef DEBUG_FX
      DEBF("Filter drive: %0.4f\r\n",drive);
#endif      
    }
    /** 
        Sets the resonance of the filter.
    */
    inline void SetResonance(float res) { res_ = res * 0.96f; }

  private:
    float istor_, res_, freq_, delay_[6], tanhstg_[3], old_freq_, old_res_, one_sr_,
        sample_rate_, acr, old_acr_, old_tune_, drive_, compens_;
    inline float my_tanh(float x);
};

inline float MoogLadder::my_tanh(float x)
{
  //return tanh(x);
    float sign = 1.0f;
    float poly;
    if (x<0.0f) {
        sign=-1.0f;
        x= -x;
    }
    if (x>=4.95f) {
      return sign;
    }
    if (x<=0.4f) return float(x*sign) * 0.9498724f; // smooth region borders    
    return  sign * Tables::lookupTable(Tables::shaper_tbl,(x*SHAPER_LOOKUP_COEF)); // lookup table, 5 is max argument value 
}

#endif