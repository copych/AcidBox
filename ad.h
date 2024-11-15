#pragma once

/**
This is an AD envelope generator made of an ADSR generator by cutting SR )))
*/


class AD_env
{
  public:
enum eSegment_t { AD_SEG_IDLE, AD_SEG_ATTACK, AD_SEG_DECAY };
    AD_env() {}
    ~AD_env() {}

    /** Initializes the Adsr module.
        \param sample_rate - The sample rate of the audio engine being run. 
    */
    void init(float sample_rate, int blockSize = 1);
	
    /**
     \function Retrigger forces the envelope back to attack phase
        \param hard resets the history to zero, results in a click.
     */
    void retrigger(bool hard = false );
	
    /**
     \function End forces the envelope to idle phase
        \param hard resets the history to zero, results in a click.
     */
    void end(bool hard);
	
    /** Processes one sample through the filter and returns one sample.
    */
    float process();

    /** 
     \function setPeakLevel sets the peak level of the envelope, it's the target level for the attack segment.
       \param peak set to 1.0 by default, in most cases you don't have to change it
    */
    inline void setPeakLevel(float peak) { attackTarget_ = peak; }
	
    /** Sets time
        Set time per segment in seconds
    */
    void setTime(int seg, float time);
    void setAttackTime(float timeInS);
    void setDecayTime(float timeInS);

    void setTimeMs(int seg, float timeInMs);
    void setAttackTimeMs(float timeInMs);
    void setDecayTimeMs(float timeInMs);
  public:
 
    /** get the current envelope segment
        \return the segment of the envelope that the phase is currently located in.
    */
    inline eSegment_t getCurrentSegment() ;
    /** Tells whether envelope is active
        \return true if the envelope is currently in any stage apart from idle.
    */
    inline bool isRunning() const { return mode_ != AD_SEG_IDLE; }
    /** Tells whether envelope is active
        \return true if the envelope is currently in any stage apart from idle.
    */
    inline bool isIdle() const { return mode_ == AD_SEG_IDLE; }

  private:
    float   sus_level_{0.f};
    volatile float  x_{0.f};
    volatile float  target_{0.f};
    volatile float 	D0_{0.f}; 
    float   peakLevel_{1.0f};
    float   attackTarget_{1.1f};
    float   attackTime_{-1.0f};
    float   decayTime_{-1.0f};
    float   attackD0_{0.f};
    float   decayD0_{0.f};
    int     sample_rate_;
    volatile eSegment_t mode_{AD_SEG_IDLE};
    bool    gate_{false};
};
