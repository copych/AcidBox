#pragma once
#ifndef DSY_MOOGLADDER_H
#define DSY_MOOGLADDER_H
 
/** Moog ladder filter module

Ported from soundpipe

Original author(s) : Victor Lazzarini, John ffitch (fast tanh), Bob Moog

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
    inline void setCutoff(float freq) { freq_ = freq; }
    /** 
        Sets the resonance of the filter.
    */
    inline void setResonance(float res) { res_ = res; }

  private:
    float istor_, res_, freq_, delay_[6], tanhstg_[3], old_freq_, old_res_,
        sample_rate_, old_acr_, old_tune_;
    float my_tanh(float x);
};
//#endif
#endif
