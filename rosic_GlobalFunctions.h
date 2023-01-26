#ifndef GlobalFunctions_h
#define GlobalFunctions_h

#include <math.h>
#include <stdlib.h>
#include "rosic_GlobalDefinitions.h"

/** This file contains a bunch of useful macros and functions which are not wrapped into the
rosic namespace to facilitate their global use. */

/** Converts a raw amplitude value/factor to a value in decibels. */
inline float amp2dB(float amp);

/** Converts a raw amplitude value/factor to a value in decibels with a check, if the amplitude is
close to zero (to avoid log-of-zero and related errors). */
inline float amp2dBWithCheck(float amp, float lowAmplitude = 0.000001);

/** Returns the index of the maximum value in an array of floats where the array should be of
length numValues. */
template <class T>
inline int arrayMaxIndex(T* theArray, int numValues);

/** Returns the index of the minimum value in an array of floats where the array should be of
length numValues. */
template <class T>
inline int arrayMinIndex(T* theArray, int numValues);

/** Converts a time-stamp given in beats into seconds acording to a tempo measured in beats per
minute (bpm). */
inline float beatsToSeconds(float beat, float bpm);

/** Converts a value in decibels to a raw amplitude value/factor. */
inline float dB2amp(float x);

/** Converts an angle in degrees into radiant. */
inline float degreeToRadiant(float degrees);

/** Frees the memory associated with the pointer ans sets the poiter itself to NULL */
//inline void deleteAndNullifyPointer(void *pointer);

/** Returns the Euclidean distance between points at coordinates (x1,y1), (x2,y2). */
inline float euclideanDistance(float x1, float y1, float x2, float y2);

/** Calculates the exponential function with base 10. */
inline float exp10(float x);

/** Calculates the exponential function with base 2. */
//inline float exp2(float x);

/** Calculates the factorial of some integer n >= 0. */
//inline int factorial(int n);

/** Converts a frequency in Hz into a MIDI-note value assuming A4 = 440 Hz. */
inline float freqToPitch(float freq);

/** Converts a frequency in Hz into a MIDI-note value for tunings different than the
default 440 Hz. */
inline float freqToPitch(float freq, float masterTuneA4);

/** Checks a pointer for nullity and if it is not NULL, it calls delete for the associated object
and then sets the pointer to NULL. */
inline void ifNotNullDeleteAndSetNull(void* pointer);

/** Maps an integer index in the range 0...numIndices-1 into a normalized floating point number in 
the range 0...1. */
inline float indexToNormalizedValue(int index, int numIndices);

/** Checks, if x is close to some target-value within some tolerance. */
inline bool isCloseTo(float x, float targetValue, float tolerance);

/** Checks, if x is even. */
inline bool isEven(int x);

/** Checks, if x is odd. */
inline bool isOdd(int x);

/** Checks, if x is a power of 2. */
inline bool isPowerOfTwo(unsigned int x);

/** Calculates the logarithm to base 2. */
//inline float log2(float x);

/** Calculates logarithm to an arbitrary base b. */
inline float logB(float x, float b);

/** Converts a value between inMin and inMax into a value between outMin and outMax where the
mapping is linear for the input and the output. Example: y = linToLin(x, 0.0, 1.0, -96.0, 24.0)
will map the input x assumed to lie inside 0.0...1.0 to the range between -96.0...24.0. This
function is useful to convert between parameter representations between 0.0...1.0 and the
clear-text parameters. */
inline float linToLin(float in, float inMin, float inMax, float outMin, float outMax);

/** Converts a value between inMin and inMax into a value between outMin and outMax where the
mapping of the output is exponential. Example: y = linToExp(x, 0.0, 1.0, 20.0, 20000.0) will map
the input x assumed to lie inside 0.0...1.0 to the range between 20.0...20000.0 where equal
differences in the input lead to equal factors in the output. Make sure that the outMin value is
greater than zero! */
inline float linToExp(float in, float inMin, float inMax, float outMin, float outMax);

/** Same as linToExp but adds an offset afterwards and compensates for that offset by scaling the
offsetted value so as to hit the outMax correctly. */
inline float linToExpWithOffset(float in, float inMin, float inMax, float outMin,
                                 float outMax, float offset = 0.0);

/** The Inverse of "linToExp" */
inline float expToLin(float in, float inMin, float inMax, float outMin, float outMax);

/** The Inverse of "linToExpWithOffset" */
inline float expToLinWithOffset(float in, float inMin, float inMax, float outMin,
                                 float outMax, float offset = 0.0);

/** Returns a power of two which is greater than or equal to the input argument. */
template <class T>
inline T nextPowerOfTwo(T x);

/** Maps a normalized floating point number in the range 0...1 into an integer index in the range 
0...numIndices-1. */
inline int normalizedValueToIndex(float normalizedValue, int numIndices);

/** Converts a picth-offset in semitones value into a frequency multiplication factor. */
inline float pitchOffsetToFreqFactor(float pitchOffset);

/** Converts a MIDI-note value into a frequency in Hz assuming A4 = 440 Hz. */
inline float pitchToFreq(float pitch);

/** Converts a MIDI-note value into a frequency in Hz for arbitrary master-tunings of A4. */
inline float pitchToFreq(float pitch, float masterTuneA4);

/** Converts an angle in radiant into degrees. */
inline float radiantToDegree(float radiant);

/** Generates a random number that is uniformly distributed between min and max (inclusive). The
underlying integer pseudo random number generator is a linear congruential with period length of 
2^32. It is based on Numerical Recipies in C (2nd edition), page 284. You may pass a seed to the 
first call to initialize it - otherwise it will use 0 as seed. A negative number (as in the default 
argument) will indicate to not initialize the state and just generate a random number based on the 
last state (which is the case for a typical call). */
inline float randomUniform(float min = 0.0, float max = 1.0, int seed = -1);

/** Returns the nearest integer (as float, without typecast). */
//inline float round(float x);

/** Converts a time value in seconds into a time value measured in beats. */
inline float secondsToBeats(float timeInSeconds, float bpm);

/** Returns the sign of x as float. */
inline float sign(float x);

/** Converts a time-stamp given in whole notes into seconds according to a tempo measured in
beats per minute (bpm). */
inline float wholeNotesToSeconds(float noteValue, float bpm);

//=================================================================================================
//implementation:

inline float amp2dB(float amp)
{
  return 8.6858896380650365530225783783321 * log(amp);
  //return 20*log10(amp); // naive version
}

inline float amp2dBWithCheck(float amp, float lowAmplitude)
{
  if( amp >= lowAmplitude )
    return 8.6858896380650365530225783783321 * log(amp);
  else
    return 8.6858896380650365530225783783321 * log(lowAmplitude);
}

template <class T>
inline int arrayMaxIndex(T* theArray, int numValues)
{
  int    maxIndex = 0;
  float maxValue = theArray[0];
  for(int i=0; i<numValues; i++)
  {
    if( theArray[i] > maxValue )
    {
      maxValue = theArray[i];
      maxIndex = i;
    }
  }
  return maxIndex;
}

template <class T>
inline int arrayMinIndex(T* theArray, int numValues)
{
  int    minIndex = 0;
  float minValue = theArray[0];
  for(int i=0; i<numValues; i++)
  {
    if( theArray[i] < minValue )
    {
      minValue = theArray[i];
      minIndex = i;
    }
  }
  return minIndex;
}

inline float beatsToSeconds(float beat, float bpm)
{
  return (60.0/bpm)*beat;
}

inline float dB2amp(float dB)
{
  return exp(dB * 0.11512925464970228420089957273422);
  //return pow(10.0, (0.05*dB)); // naive, inefficient version
}

inline float degreeToRadiant(float degrees)
{
  return (3.1415926535897932384626433832795f/180.0)*degrees;
}

/*
inline void deleteAndNullifyPointer(void *pointer)
{
  delete pointer;
  pointer = NULL;
}
*/

inline float euclideanDistance(float x1, float y1, float x2, float y2)
{
  return sqrt( (x2-x1)*(x2-x1) + (y2-y1)*(y2-y1) );
}

inline float exp10(float x)
{
  return exp(LN10*x);
}
/*
inline float exp2(float x)
{
  return exp(LN2*x);
}
*/
inline float freqToPitch(float freq)
{
  return 12.0 * log2(freq/440.0) + 69.0;
}

inline float freqToPitch(float freq, float masterTuneA4)
{
  return 12.0 * log2(freq/masterTuneA4) + 69.0;
}

/*
inline void ifNotNullDeleteAndSetNull(void* pointer)
{
  if( pointer != NULL )
  {
    delete pointer;
    pointer = NULL;
  }
}
*/

inline float indexToNormalizedValue(int index, int numIndices)
{
  return (float) (2*index+1) / (float) (2*numIndices);
}

inline bool isCloseTo(float x, float targetValue, float tolerance)
{
  if( fabs(x-targetValue) <= tolerance )
    return true;
  else
    return false;
}

inline bool isEven(int x)
{
  if( x%2 == 0 )
    return true;
  else
    return false;
}

inline bool isOdd(int x)
{
  if( x%2 != 0 )
    return true;
  else
    return false;
}

inline bool isPowerOfTwo(unsigned int x)
{
  unsigned int currentPower = 1;
  while( currentPower <= x )
  {
    if( currentPower == x )
      return true;
    currentPower *= 2;
  }
  return false;
}
/*
inline float log2(float x)
{
  return ONE_OVER_LN2*log(x);
}
*/
inline float logB(float x, float b)
{
  return log(x)/log(b);
}

inline float linToLin(float in, float inMin, float inMax, float outMin, float outMax)
{
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) / (inMax-inMin);

  // map the tmp-value to the range outMin...outMax:
  tmp *= (outMax-outMin);
  tmp += outMin;

  return tmp;
}

inline float linToExp(float in, float inMin, float inMax, float outMin, float outMax)
{
  // map input to the range 0.0...1.0:
  float tmp = (in-inMin) / (inMax-inMin);

  // map the tmp-value exponentially to the range outMin...outMax:
  //tmp = outMin * exp( tmp*(log(outMax)-log(outMin)) );
  return outMin * exp( tmp*(log(outMax/outMin)) );
}

inline float linToExpWithOffset(float in, float inMin, float inMax, float outMin,
                                 float outMax, float offset)
{
  float tmp = linToExp(in, inMin, inMax, outMin, outMax);
  tmp += offset;
  tmp *= outMax/(outMax+offset);
  return tmp;
}

inline float expToLin(float in, float inMin, float inMax, float outMin, float outMax)
{
  float tmp = log(in/inMin) / log(inMax/inMin);
  return outMin + tmp * (outMax-outMin);
}

inline float expToLinWithOffset(float in, float inMin, float inMax, float outMin,
                                 float outMax, float offset)
{
  float tmp = in*(inMax+offset)/inMax;
  tmp -= offset;
  return expToLin(tmp, inMin, inMax, outMin, outMax);
  /*
  float tmp = linToExp(in, inMin, inMax, outMin, outMax);
  tmp += offset;
  tmp *= outMax/(outMax+offset);
  return tmp;
  */
}

template <class T>
inline T nextPowerOfTwo(T x)
{
  T accu = 1;
  while(accu < x)
    accu *= 2;
  return accu;
}

inline int normalizedValueToIndex(float normalizedValue, int numIndices)
{
  return (int) floor(normalizedValue*numIndices);
}

inline float pitchOffsetToFreqFactor(float pitchOffset)
{
  return exp(0.057762265046662109118102676788181 * pitchOffset);
  //return pow(2.0, pitchOffset/12.0); // naive, slower but numerically more precise
}

inline float pitchToFreq(float pitch)
{
  return 8.1757989156437073336828122976033 * exp(0.057762265046662109118102676788181*pitch);
  //return 440.0*( pow(2.0, (pitch-69.0)/12.0) ); // naive, slower but numerically more precise
}

inline float pitchToFreq(float pitch, float masterTuneA4)
{
  return masterTuneA4 * 0.018581361171917516667460937040007
    * exp(0.057762265046662109118102676788181*pitch);
}

inline float radiantToDegree(float radiant)
{
  return (180.0/3.1415926535897932384626433832795f)*radiant;
}

inline float randomUniform(float min, float max, int seed)
{
  static unsigned long state = 0;
  if( seed >= 0 )
    state = seed;                                        // initialization, if desired
  state = 1664525*state + 1013904223;                    // mod implicitely by integer overflow
  return min + (max-min) * ((1.0/4294967296.0) * state); // transform to desired range
}
/*
inline float round(float x)
{
  if( x-floor(x) >= 0.5 )
    return ceil(x);
  else
    return floor(x);
}
*/
inline float secondsToBeats(float timeInSeconds, float bpm)
{
  return timeInSeconds*(bpm/60.0);
}

inline float sign(float x)
{
  if(x<0)
    return -1.0;
  else if(x>0)
    return 1.0;
  else
    return 0;
}

inline float wholeNotesToSeconds(float noteValue, float bpm)
{
  return (240.0/bpm)*noteValue;
}

#endif // #ifndef GlobalFunctions_h
