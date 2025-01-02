#pragma once
#ifndef DSY_WAVEFOFOLDER_H
#define DSY_WAVEFOFOLDER_H

#include <math.h>

/** Basic wavefolder module.

Amplitude of input determines level of folding.
Amplitudes of magnitude > 1.0 will start to fold.

Original author(s) : Nick Donaldson
Year : 2022
*/
class Wavefolder {
  public:
    Wavefolder() {}
    ~Wavefolder() {}
    /** Initializes the wavefolder module.
    */
    void Init();
    /** applies wavefolding to input 
    */
    float Process(float in);
    /** 
        \param gain Set input gain.
        Supports negative values for thru-zero
    */
    void SetDrive(float gain);
    /** 
        \param offset Offset odded to input (pre-gain) for asymmetrical folding.
    */
    void SetOffset(float offset) { offset_ = offset; }

  private:
    float gain_, offset_, compens_;
};

#endif
