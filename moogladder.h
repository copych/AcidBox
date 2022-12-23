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
    inline void SetCutoff(float freq) { freq_ = freq; }

    inline void SetDrive(float drive) { drive_ = drive; }
    /** 
        Sets the resonance of the filter.
    */
    inline void SetResonance(float res) { res_ = res; }

  private:
    float istor_, res_, freq_, delay_[6], tanhstg_[3], old_freq_, old_res_, one_sr_,
        sample_rate_, acr, old_acr_, old_tune_, drive_, resQ_;
    float my_tanh(float x);
};
//#endif
#endif
