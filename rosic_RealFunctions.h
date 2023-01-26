#ifndef rosic_RealFunctions_h
#define rosic_RealFunctions_h

// standard library includes:
#include <math.h>
//#include <stdlib.h>

// rosic includes:
#include "rosic_GlobalFunctions.h"
#include "rosic_NumberManipulations.h"
 
namespace rosic
{

  /** Inverse hyperbolic sine. */
  inline float asinh(float x);

  /** Returns -1.0 if x is below low, 0.0 if x is between low and high and 1.0 if x is above high. */
  inline float belowOrAbove(float x, float low, float high);

  /** Clips x into the range min...max. */
  template <typename T>
  inline T clip(T x, T min, T max);

  /** Evaluates the quartic polynomial y = a4*x^4 + a3*x^3 + a2*x^2 + a1*x + a0 at x. */
  inline float evaluateQuartic(float x, float a0, float a1, float a2, float a3, float a4);

  /** foldover at the specified value */
  inline float foldOver(float x, float min, float max);

  /** Computes an integer power of x by successively multiplying x with itself. */
  inline float integerPower(float x, int exponent);

  /** Generates a pseudo-random number between min and max. */
  inline float random(float min=0.0, float max=1.0);

  /** Generates a 2*pi periodic saw wave. */
  inline float sawWave(float x);

  /** Calculates sine and cosine of x - this is more efficient than calling sin(x) and
  cos(x) seperately. */
  inline void sinCos(float x, float* sinResult, float* cosResult);

  /** Calculates a parabolic approximation of the sine and cosine of x. */
  inline void sinCosApprox(float x, float* sinResult, float* cosResult);

  /** Generates a 2*pi periodic square wave. */
  inline float sqrWave(float x);

  /** Rational approximation of the hyperbolic tangent. */
  inline float tanhApprox(float x);

  /** Generates a 2*pi periodic triangle wave. */
  inline float triWave(float x);

  //===============================================================================================
  // implementation:

  inline float asinh(float x)
  {
    return log(x + sqrt(x*x+1) );
  }

  inline float belowOrAbove(float x, float low, float high)
  {
    if( x < low )
      return -1.0;
    else if ( x > high )
      return 1.0;
    else
      return 0.0;
  }

  template <class T>
  inline T clip(T x, T min, T max)
  {
    if( x > max )
      return max;
    else if ( x < min )
      return min;
    else return x;
  }

  inline float evaluateQuartic(float x, float a0, float a1, float a2, float a3, float a4)
  {
    float x2 = x*x;
    return x*(a3*x2+a1) + x2*(a4*x2+a2) + a0;
  }

  inline float foldOver(float x, float min, float max)
  {
    if( x > max )
      return max - (x-max);
    else if( x < min )
      return min - (x-min);
    else return x;
  }

  inline float integerPower(float x, int exponent)
  {
    float accu = 1.0;
    for(int i=0; i<exponent; i++)
      accu *= x;
    return accu;
  }

  inline float random(float min, float max)
  {
    float tmp = (1.0/RAND_MAX) * rand() ;  // between 0...1
    return linToLin(tmp, 0.0, 1.0, min, max);
  }

  inline float sawWave(float x)
  {
    float tmp = fmod(x, 2*PI);
    if( tmp < PI )
      return tmp/PI;
    else
      return (tmp/PI)-2.0;
  }

  inline void sinCos(float x, float* sinResult, float* cosResult)
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

  inline void sinCosApprox(float x, float* sinResult, float* cosResult)
  {
    static const float c = 0.70710678118654752440;

    // restrict input x to the range 0.0...2*PI:
    while( x > 2.0*PI )
      x -= 2*PI;
    while( x < 0.0 )
      x += 2*PI;

    if( x < PI/2 )
    {
      float tmp1 = x;
      float tmp2 = (2/PI) * tmp1 - 0.5;
      float tmp3 = (2-4*c)*tmp2*tmp2 + c;
      *sinResult  = tmp3 + tmp2;
      *cosResult  = tmp3 - tmp2;
    }
    else if( x < PI )
    {
      float tmp1 = (x-PI/2);
      float tmp2 = 0.5 - (2/PI) * tmp1;
      float tmp3 = (2-4*c)*tmp2*tmp2 + c;
      *sinResult  = tmp2 + tmp3;
      *cosResult  = tmp2 - tmp3;
    }
    else if( x < 1.5*PI )
    {
      float tmp1 = (x-PI);
      float tmp2 = (2/PI) * tmp1 - 0.5;
      float tmp3 = (4*c-2)*tmp2*tmp2 - c;
      *sinResult  = tmp3 - tmp2;
      *cosResult  = tmp3 + tmp2;
    }
    else
    {
      float tmp1 = (x-1.5*PI);
      float tmp2 = (2/PI) * tmp1 - 0.5;
      float tmp3 = (2-4*c)*tmp2*tmp2 + c;
      *sinResult  = tmp2 - tmp3;
      *cosResult  = tmp2 + tmp3;
    }
  }

  inline float sqrWave(float x)
  {
    float tmp = fmod(x, 2*PI);
    if( tmp < PI )
      return 1.0;
    else
      return -1.0;
  }

  inline float tanhApprox(float x)
  {
    float a = fabs(2*x);
    float b = 24+a*(12+a*(6+a));
    return 2*(x*b)/(a*b+48);
  }

  inline float triWave(float x)
  {
    float tmp = fmod(x, 2*PI);
    if( tmp < 0.5*PI )
      return tmp/(0.5*PI);
    else if( tmp < 1.5*PI )
      return 1.0 - ((tmp-0.5*PI)/(0.5*PI));
    else
      return -1.0 + ((tmp-1.5*PI)/(0.5*PI));
  }

} // end namespace rosic

#endif // #ifndef rosic_RealFunctions_h
