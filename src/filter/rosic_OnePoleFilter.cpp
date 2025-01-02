#include "rosic_OnePoleFilter.h"

//-------------------------------------------------------------------------------------------------
// construction/destruction:

OnePoleFilter::OnePoleFilter()
{
  shelvingGain = 1.0f;
  setSampleRate((float)SAMPLE_RATE);  // sampleRate = 44100 Hz by default
  setMode      (0);        // bypass by default
  setCutoff    (100.0f);  // cutoff = 100 Hz by default
  reset();                 // reset memorized samples to zero
}

//-------------------------------------------------------------------------------------------------
// parameter settings:

void OnePoleFilter::setSampleRate(float newSampleRate)
{
  if( newSampleRate > 0.0f )
    sampleRate = newSampleRate;
  sampleRateRec = 1.0f / sampleRate;

  calcCoeffs();
  return;
}

void OnePoleFilter::setMode(int newMode)
{
  mode = newMode; // 0:bypass, 1:Low Pass, 2:High Pass
  calcCoeffs();
}

void OnePoleFilter::setCutoff(float newCutoff)
{
  if( (newCutoff>0.0f) && (newCutoff<=20000.0f) )
    cutoff = newCutoff;
  else
    cutoff = 20000.0f;

  calcCoeffs();
  return;
}

void OnePoleFilter::setShelvingGain(float newGain)
{
  if( newGain > 0.0f )
  {
    shelvingGain = newGain;
    calcCoeffs();
  }
  else 
  {
  }
}

void OnePoleFilter::setShelvingGainInDecibels(float newGain)
{
  setShelvingGain(exp(newGain * 0.11512925464970228420089957273422f));
}

void OnePoleFilter::setCoefficients(float newB0, float newB1, float newA1)
{
  b0 = newB0;
  b1 = newB1;
  a1 = newA1;
}

void OnePoleFilter::setInternalState(float newX1, float newY1)
{
  x1 = newX1;
  y1 = newY1;
}

//-------------------------------------------------------------------------------------------------
//others:

void OnePoleFilter::calcCoeffs()
{
  switch(mode)
  {
  case LOWPASS: 
    {
      // formula from dspguide:
      float x = exp( -2.0f * PI * cutoff * sampleRateRec); 
      b0 = 1.0f-x;
      b1 = 0.0f;
      a1 = x;
    }
    break;
  case HIGHPASS:  
    {
      // formula from dspguide:
      float x = exp( -2.0f * PI * cutoff * sampleRateRec);
      b0 =  0.5f*(1.0f+x);
      b1 = -0.5f*(1.0f+x);
      a1 = x;
    }
    break;
  case LOWSHELV:
    {
      // formula from DAFX:
      float c = 0.5f*(shelvingGain-1.0f);
      float t = tan(PI*cutoff*sampleRateRec);
      float a;
      if( shelvingGain >= 1.0f )
        a = (t-1.0f)/(t+1.0f);
      else
        a = (t-shelvingGain)/(t+shelvingGain);

      b0 = 1.0f + c + c*a;
      b1 = c + c*a + a;
      a1 = -a;
    }
    break;
  case HIGHSHELV:
    {
      // formula from DAFX:
      float c = 0.5f*(shelvingGain-1.0f);
      float t = tan(PI*cutoff*sampleRateRec);
      float a;
      if( shelvingGain >= 1.0f )
        a = (t-1.0f)/(t+1.0f);
      else
        a = (shelvingGain*t-1.0f)/(shelvingGain*t+1.0f);

      b0 = 1.0f + c - c*a;
      b1 = a + c*a - c;
      a1 = -a;
    }
    break;

  case ALLPASS:  
    {
      // formula from DAFX:
      float t = tan(PI*cutoff*sampleRateRec);
      float x = (t-1.0f) / (t+1.0f);

      b0 = x;
      b1 = 1.0f;
      a1 = -x;
    }
    break;

  default: // bypass
    {
      b0 = 1.0f;
      b1 = 0.0f;
      a1 = 0.0f;
    }break;
  }
}

void OnePoleFilter::reset()
{
  x1 = 0.0f;
  y1 = 0.0f;
}
