#include "rosic_TeeBeeFilter.h"

//-------------------------------------------------------------------------------------------------
// construction/destruction:

TeeBeeFilter::TeeBeeFilter()
{
  cutoff              =  1000.0f;
  drive               =     0.1f;
  driveFactor         =     1.0f;
  resonanceRaw        =     0.0f;
  resonanceSkewed     =     0.0f;
  g                   =     1.0f;
  sampleRate          = SAMPLE_RATE;
  twoPiOverSampleRate = 2.0f*PI/sampleRate;

  feedbackHighpass.setMode(OnePoleFilter::HIGHPASS);
  feedbackHighpass.setCutoff(100.0f);

 // SetMode(LP_18);
  SetMode(TB_303);
  calculateCoefficientsExact();
  Init();
}


TeeBeeFilter::~TeeBeeFilter()
{
}

//-------------------------------------------------------------------------------------------------
// parameter settings:

void TeeBeeFilter::SetSampleRate(float newSampleRate)
{
  if( newSampleRate > 0.0 )
    sampleRate = newSampleRate;
  twoPiOverSampleRate = 2.0*PI/sampleRate;
  highLimit = sampleRate / 4.18f;
  feedbackHighpass.setSampleRate(newSampleRate);
  calculateCoefficientsExact();
}


void TeeBeeFilter::SetMode(int newMode)
{
  if( newMode >= 0 && newMode < NUM_MODES )
  {
    mode = newMode;
    switch(mode)
    {
    case FLAT:      c0 =  1.0f; c1 =  0.0f; c2 =  0.0f; c3 =  0.0f; c4 =  0.0f;  break;
    case LP_6:      c0 =  0.0f; c1 =  1.0f; c2 =  0.0f; c3 =  0.0f; c4 =  0.0f;  break;
    case LP_12:     c0 =  0.0f; c1 =  0.0f; c2 =  1.0f; c3 =  0.0f; c4 =  0.0f;  break;
    case LP_18:     c0 =  0.0f; c1 =  0.0f; c2 =  0.0f; c3 =  1.0f; c4 =  0.0f;  break;
    case LP_24:     c0 =  0.0f; c1 =  0.0f; c2 =  0.0f; c3 =  0.0f; c4 =  1.0f;  break;
    case HP_6:      c0 =  1.0f; c1 = -1.0f; c2 =  0.0f; c3 =  0.0f; c4 =  0.0f;  break;
    case HP_12:     c0 =  1.0f; c1 = -2.0f; c2 =  1.0f; c3 =  0.0f; c4 =  0.0f;  break;
    case HP_18:     c0 =  1.0f; c1 = -3.0f; c2 =  3.0f; c3 = -1.0f; c4 =  0.0f;  break;
    case HP_24:     c0 =  1.0f; c1 = -4.0f; c2 =  6.0f; c3 = -4.0f; c4 =  1.0f;  break;
    case BP_12_12:  c0 =  0.0f; c1 =  0.0f; c2 =  1.0f; c3 = -2.0f; c4 =  1.0f;  break;
    case BP_6_18:   c0 =  0.0f; c1 =  0.0f; c2 =  0.0f; c3 =  1.0f; c4 = -1.0f;  break;
    case BP_18_6:   c0 =  0.0f; c1 =  1.0f; c2 = -3.0f; c3 =  3.0f; c4 = -1.0f;  break;
    case BP_6_12:   c0 =  0.0f; c1 =  0.0f; c2 =  1.0f; c3 = -1.0f; c4 =  0.0f;  break;
    case BP_12_6:   c0 =  0.0f; c1 =  1.0f; c2 = -2.0f; c3 =  1.0f; c4 =  0.0f;  break;
    case BP_6_6:    c0 =  0.0f; c1 =  1.0f; c2 = -1.0f; c3 =  0.0f; c4 =  0.0f;  break;
    default:        c0 =  1.0f; c1 =  0.0f; c2 =  0.0f; c3 =  0.0f; c4 =  0.0f;  // flat
    }
  }
  calculateCoefficientsApprox4();
}



//-------------------------------------------------------------------------------------------------
// others:


//-----------------------------------------------------------------------------------------------
// inlined functions:

inline void TeeBeeFilter::SetCutoff(float newCutoff, bool updateCoefficients)
{
  if ( newCutoff != cutoff )
  {
    if ( newCutoff < 200.0f ) // an absolute floor for the cutoff frequency - tweakable
      cutoff = 200.0f;
    else if ( newCutoff > highLimit ) // seems to be stable with the current settings, higher values may lead to nan, +inf or -inf during processing
      cutoff = highLimit;
    else
      cutoff = newCutoff;

    if ( updateCoefficients == true )
      calculateCoefficientsApprox4();
  }
}

inline void TeeBeeFilter::SetResonance(float newResonance, bool updateCoefficients)
{
  resonanceRaw    =  newResonance;
//  compens = 1.8f * (resonanceRaw + 0.25f) * one_div((resonanceRaw + 0.25f) * 0.75f + 0.113f); // gain compensation; one_div(x) = 1/x
  resonanceSkewed = 1.02f * (1.0f - exp(-3.0f * resonanceRaw)) / (1.0f - exp(-3.0f));
//  DEBF("%f\r\n",resonanceSkewed);
  if ( updateCoefficients == true )
    calculateCoefficientsApprox4();
//    calculateCoefficientsExact();
}


void TeeBeeFilter::SetDrive(float newDrive)
{
  drive       = newDrive + 0.01f;

}


inline void TeeBeeFilter::calculateCoefficientsExact()
{
  // calculate intermediate variables:
  float wc = twoPiOverSampleRate * cutoff;
  float s, c;
  //sinCos(wc, &s, &c);             // c = cos(wc); s = sin(wc);
  fast_sincos(wc, &s, &c);             // c = cos(wc); s = sin(wc);
  float t  = tan(0.25f * (wc - PI));
  float r  = resonanceSkewed;

  // calculate filter a1-coefficient tuned such the resonance frequency is just right:
  float a1_fullRes = t * one_div(s - c * t);

  // calculate filter a1-coefficient as if there were no resonance:
  float x        = exp(-wc);
  float a1_noRes = -x;

  // use a weighted sum between the resonance-tuned and no-resonance coefficient:
  a1 = r * a1_fullRes + (1.0f - r) * a1_noRes;
  // calculate the b0-coefficient from the condition that each stage should be a leaky
  // integrator:
  b0 = 1.0f + a1;

  // calculate feedback factor by dividing the resonance parameter by the magnitude at the
  // resonant frequency:
  float gsq = b0 * b0 * one_div(1.0f + a1 * a1 + 2.0f * a1 * c);
  k          = r * one_div(gsq * gsq);

  if ( mode == TB_303 )
    k *= (4.25f);
}

inline void TeeBeeFilter::calculateCoefficientsApprox4()
{
  // calculate intermediate variables:
  float wc  = twoPiOverSampleRate * cutoff;
  float wc2 = wc * wc;
  float r   = resonanceSkewed;
  float tmp;

  // compute the filter coefficient via a 12th order polynomial approximation (polynomial
  // evaluation is done with a Horner-rule alike scheme with nested quadratic factors in the hope
  // for potentially better parallelization compared to Horner's rule as is):
  const float pa12 = -1.341281325101042e-02;
  const float pa11 =  8.168739417977708e-02;
  const float pa10 = -2.365036766021623e-01;
  const float pa09 =  4.439739664918068e-01;
  const float pa08 = -6.297350825423579e-01;
  const float pa07 =  7.529691648678890e-01;
  const float pa06 = -8.249882473764324e-01;
  const float pa05 =  8.736418933533319e-01;
  const float pa04 = -9.164580250284832e-01;
  const float pa03 =  9.583192455599817e-01;
  const float pa02 = -9.999994950291231e-01;
  const float pa01 =  9.999999927726119e-01;
  const float pa00 = -9.999999999857464e-01;
  tmp  = wc2 * pa12 + pa11 * wc + pa10;
  tmp  = wc2 * tmp  + pa09 * wc + pa08;
  tmp  = wc2 * tmp  + pa07 * wc + pa06;
  tmp  = wc2 * tmp  + pa05 * wc + pa04;
  tmp  = wc2 * tmp  + pa03 * wc + pa02;
  a1   = wc2 * tmp  + pa01 * wc + pa00;
  b0   = 1.0f + a1;

  // compute the scale factor for the resonance parameter (the factor to obtain k from r) via an
  // 8th order polynomial approximation:
  const float pr8 = -4.554677015609929e-05;
  const float pr7 = -2.022131730719448e-05;
  const float pr6 =  2.784706718370008e-03;
  const float pr5 =  2.079921151733780e-03;
  const float pr4 = -8.333236384240325e-02;
  const float pr3 = -1.666668203490468e-01;
  const float pr2 =  1.000000012124230e+00;
  const float pr1 =  3.999999999650040e+00;
  const float pr0 =  4.000000000000113e+00;
  tmp  = wc2 * pr8 + pr7 * wc + pr6;
  tmp  = wc2 * tmp + pr5 * wc + pr4;
  tmp  = wc2 * tmp + pr3 * wc + pr2;
  tmp  = wc2 * tmp + pr1 * wc + pr0; // this is now the scale factor
  k    = r * tmp;
  g    = 1.0f;

  if ( mode == TB_303 )
  {
    float fx = wc * ONE_DIV_SQRT2 * ONE_DIV_TWOPI;
    b0 = (0.00045522346f + 6.1922189f * fx) * one_div(1.0f + 12.358354f * fx + 4.4156345f * (fx * fx));
    k  = fx * (fx * (fx * (fx * (fx * (fx + 7198.6997f) - 5837.7917f) - 476.47308f) + 614.95611f) + 213.87126f) + 16.998792f;
    g  = k * 0.05882352f; // 17 reciprocal
    g  = (g - 1.0f) * r + 1.0f;                     // r is 0 to 1.0
    g  = (g * (1.0f + r));
    k  = k * r;                                   // k is ready now
  }
}

inline float TeeBeeFilter::Process(float in)
{
  float y0;

    if( mode == TB_303 )
    {
      //y0  = in - feedbackHighpass.getSample(k * shape(y4));  
      y0 = in - feedbackHighpass.getSample(k*y4);  
      //y0  = in - k*shape(y4);  
      //y0  = in-k*y4;  
      y1 += 2*b0*(y0-y1+y2);
      y2 +=   b0*(y1-2*y2+y3);
      y3 +=   b0*(y2-2*y3+y4);
      y4 +=   b0*(y3-2*y4);
      return 2*g*y4;
      //return 3*y4;
    }

    // apply drive and feedback to obtain the filter's input signal:
    //float y0 = inputFilter.getSample(0.125*driveFactor*in) - feedbackHighpass.getSample(k*y4);
    y0 = 0.125f * driveFactor * in - feedbackHighpass.getSample(k*y4);  
    
    y1 = y0 + a1*(y0-y1);
    y2 = y1 + a1*(y1-y2);
    y3 = y2 + a1*(y2-y3);
    y4 = y3 + a1*(y3-y4); // \todo: performance test both versions of the ladder
    //y4 = shape(y3 + a1*(y3-y4)); // \todo: performance test both versions of the ladder

    return 8.0f * (c0*y0 + c1*y1 + c2*y2 + c3*y3 + c4*y4);;
}

inline void TeeBeeFilter::sinCos(float x, float* sinResult, float* cosResult)
{
#ifdef __GNUC__  // \todo assembly-version causes compiler errors on gcc
  *sinResult = sin(x);
  *cosResult = cos(x);
#else
  float s, c;     // do we need these intermediate variables?
  __asm fld x
  __asm fsincos
  __asm fstp c
  __asm fstp s
  *sinResult = s;
  *cosResult = c;
#endif
}


inline float TeeBeeFilter::shape(float x)
  {
    // return tanhApprox(x); // \todo: find some more suitable nonlinearity here
    //return x; // test

   // const float r6 = 1.0/6.0;
  //  x = clip<float>(x, -SQRT2, SQRT2);
  //  return x - r6*x*x*x;

    //return clip(x, -1.0, 1.0);
    return fast_shape(x*2.0f);
  }


void TeeBeeFilter::Init() {
  feedbackHighpass.reset();
  y1 = 0.0f;
  y2 = 0.0f;
  y3 = 0.0f;
  y4 = 0.0f;
  SetDrive(0.11f);
  SetResonance(0.5f, false);
  SetCutoff(1000.0f, true);
}


void TeeBeeFilter::Init(float sr)
{
  feedbackHighpass.reset();
  SetSampleRate(sr);
  y1 = 0.0f;
  y2 = 0.0f;
  y3 = 0.0f;
  y4 = 0.0f;
  SetDrive(0.11f);
  SetResonance(0.5f, false);
  SetCutoff(1000.0f, true);
}
