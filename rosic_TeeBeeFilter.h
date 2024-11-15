#ifndef rosic_TeeBeeFilter_h
#define rosic_TeeBeeFilter_h

#define SQRT2 1.4142135623730950488016887242097f
#define ONE_DIV_SQRT2 0.70710678118654752440084436210485f

#include "rosic_OnePoleFilter.h"

/**
  This class is a filter that aims to emulate the filter in the Roland TB 303. It's a variation of
  the Moog ladder filter which includes a highpass in the feedback path that reduces the resonance
  on low cutoff frequencies. Moreover, it has a highpass and an allpass filter in the input path to
  pre-shape the input signal (important for the sonic character of internal and subsequent
  nonlinearities).

  ...18 vs. 24 dB? blah?
*/

class TeeBeeFilter
{

  public:

    /** Enumeration of the available filter modes. */
    enum modes
    {
      FLAT = 0,
      LP_6,
      LP_12,
      LP_18,
      LP_24,
      HP_6,
      HP_12,
      HP_18,
      HP_24,
      BP_12_12,
      BP_6_18,
      BP_18_6,
      BP_6_12,
      BP_12_6,
      BP_6_6,
      TB_303,      // ala mystran & kunn (page 40 in the kvr-thread)

      NUM_MODES
    };

    //---------------------------------------------------------------------------------------------
    // construction/destruction:

    /** Constructor. */
    TeeBeeFilter();

    /** Destructor. */
    ~TeeBeeFilter();

    //---------------------------------------------------------------------------------------------
    // parameter settings:

    /** Sets the sample-rate for this filter. */
    void SetSampleRate(float newSampleRate);

    /** Sets the cutoff frequency for this filter - the actual coefficient calculation may be
      supressed by passing 'false' as second parameter, in this case, it should be triggered
      manually later by calling calculateCoefficients. */
    inline void SetCutoff(float newCutoff, bool updateCoefficients = true);

    /** Sets the resonance in percent where 100% is self oscillation. */
    inline void SetResonance(float newResonance, bool updateCoefficients = true);

    /** Sets the input drive in decibels. */
    void SetDrive(float newDrive);

    /** Sets the mode of the filter, @see: modes */
    void SetMode(int newMode);

    /** Sets the cutoff frequency for the highpass filter in the feedback path. */
    void SetFeedbackHighpassCutoff(float newCutoff) {
      feedbackHighpass.setCutoff(newCutoff);
    }

    //---------------------------------------------------------------------------------------------
    // inquiry:

    /** Returns the cutoff frequency of this filter. */
    float GetCutoff() const {
      return cutoff;
    }

    /** Returns the resonance parameter of this filter. */
    float GetResonance() const {
      return 100.0 * resonanceRaw;
    }

    /** Returns the drive parameter in decibels. */
    float GetDrive() const {
      return drive;
    }

    /** Returns the slected filter mode. */
    int GetMode() const {
      return mode;
    }

    /** Returns the cutoff frequency for the highpass filter in the feedback path. */
    float GetFeedbackHighpassCutoff() const {
      return feedbackHighpass.getCutoff();
    }

    //---------------------------------------------------------------------------------------------
    // audio processing:

    /** Calculates one output sample at a time. */
    inline float Process(float in);

    //---------------------------------------------------------------------------------------------
    // others:


    /** Causes the filter to re-calculate the coeffiecients via the exact formulas. */
    inline void calculateCoefficientsExact();

    /** Causes the filter to re-calculate the coeffiecients using an approximation that is valid
      for normalized radian cutoff frequencies up to pi/4. */
    inline void calculateCoefficientsApprox4();

    /** Calculates sine and cosine of x - this is more efficient than calling sin(x) and
      cos(x) seperately. */
    inline void sinCos(float x, float* sinResult, float* cosResult);

    /** Implements the waveshaping nonlinearity between the stages. */
    inline float shape(float x);

    /** Implements limitting of a given value */
    inline float fclamp(float in, float min, float max){
      if (in>max) return max;
      if (in<min) return min;
      return in;
    };

    /** Resets the internal state variables. */
    void Init();

    void Init(float sr);
    //=============================================================================================

  protected:

    float b0, a1;              // coefficients for the first order sections
    float y1, y2, y3, y4;      // output signals of the 4 filter stages
    float c0, c1, c2, c3, c4;  // coefficients for combining various ouput stages
    float k;                   // feedback factor in the loop
    float g;                   // output gain
    float driveFactor;         // filter drive as raw factor
    float cutoff;              // cutoff frequency
    float drive;               // filter drive normalized
    float resonanceRaw;        // resonance parameter (normalized to 0...1)
    float resonanceSkewed;     // mapped resonance parameter to make it behave more musical
    float sampleRate;          // the sample rate in Hz
    float twoPiOverSampleRate; // 2*PI/sampleRate
    float highLimit;           // highest cutoff freq allowing stable behaviour
    float compens;             // drive gain correction
    float bias;                 // reso bias compensation
    int    mode;                // the selected filter-mode

    OnePoleFilter feedbackHighpass;

};


#endif
