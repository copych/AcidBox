#include "krajeski_flt.h"


/*
This class implements Tim Stilson's MoogVCF filter
using 'compromise' poles at z = -0.3

Several improments are built in, such as corrections
for cutoff and resonance parameters, removal of the
necessity of the separation table, audio rate update
of cutoff and resonance and a smoothly saturating
tanh() function, clamping output and creating inherent
nonlinearities.

This code is Unlicensed (i.e. public domain); in an email exchange on
4.21.2018 Aaron Krajeski stated: "That work is under no copyright. 
You may use it however you might like."

Source: http://song-swap.com/MUMT618/aaron/Presentation/demo.html

Modified by Copych 2023: added fclamp because sometimes nan, -inf, +inf appears in state[] of Process()

*/

  float KrajeskiMoog::Process(float sample) 
  {
      state[0] = fast_shape(drive * (sample - 4.0f * gRes * (state[4] - gComp * sample)));
      
      for(int i = 0; i < 4; i++)
      {
        state[i+1] = General::fclamp(g * ( 0.2307692f * state[i] + 0.7692307f * delay[i] - state[i + 1]) + state[i + 1], -1e30, 1e30 );
        delay[i] = state[i];

      }
      return state[4];
  }
  
