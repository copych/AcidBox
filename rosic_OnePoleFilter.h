#ifndef rosic_OnePoleFilter_h
#define rosic_OnePoleFilter_h

/**
  This is an implementation of a simple one-pole filter unit.
*/

class OnePoleFilter
{
  public:

    /** This is an enumeration of the available filter modes. */
    enum modes
    {
      BYPASS = 0,
      LOWPASS,
      HIGHPASS,
      LOWSHELV,
      HIGHSHELV,
      ALLPASS
    };
    // \todo (maybe): let the user choose between LP/HP versions obtained via bilinear trafo and
    // impulse invariant trafo

    //---------------------------------------------------------------------------------------------
    // construction/destruction:

    /** Constructor. */
    OnePoleFilter();

    //---------------------------------------------------------------------------------------------
    // parameter settings:

    /** Sets the sample-rate. */
    void setSampleRate(float newSampleRate);

    /** Chooses the filter mode. See the enumeration for available modes. */
    void setMode(int newMode);

    /** Sets the cutoff-frequency for this filter. */
    void setCutoff(float newCutoff);

    /** This will set the time constant 'tau' for the case, when lowpass mode is chosen. This is
      the time, it takes for the impulse response to die away to 1/e = 0.368... or equivalently, the
      time it takes for the step response to raise to 1-1/e = 0.632... */
    void setLowpassTimeConstant(float newTimeConstant) {
      setCutoff(1.0 / (2 * PI * newTimeConstant));
    }

    /** Sets the gain factor for the shelving modes (this is not in decibels). */
    void setShelvingGain(float newGain);

    /** Sets the gain for the shelving modes in decibels. */
    void setShelvingGainInDecibels(float newGain);

    /** Sets the filter coefficients manually. */
    void setCoefficients(float newB0, float newB1, float newA1);

    /** Sets up the internal state variables for both channels. */
    void setInternalState(float newX1, float newY1);

    //---------------------------------------------------------------------------------------------
    // inquiry

    /** Returns the cutoff-frequency. */
    float getCutoff() const {
      return cutoff;
    }

    //---------------------------------------------------------------------------------------------
    // audio processing:

    /** Calculates a single filtered output-sample. */
    inline float getSample(float in);

    //---------------------------------------------------------------------------------------------
    // others:

    /** Resets the internal buffers (for the \f$ x[n-1], y[n-1] \f$-samples) to zero. */
    void reset();

    //=============================================================================================

  protected:

    // buffering:
    float x1, y1;

    // filter coefficients:
    float b0; // feedforward coeffs
    float b1;
    float a1; // feedback coeff

    // filter parameters:
    float cutoff;
    float shelvingGain;
    int    mode;

    float sampleRate;
    float sampleRateRec;  // reciprocal of the sampleRate

    // internal functions:
    void calcCoeffs();  // calculates filter coefficients from filter parameters

};

//-----------------------------------------------------------------------------------------------
// inlined functions:

inline float OnePoleFilter::getSample(float in)
{
  // calculate the output sample:
  y1 = (float)b0 * in + b1 * x1 + a1 * y1 + (float)1.1e-38;

  // update the buffer variables:
  x1 = in;

  return y1;
}
#endif
