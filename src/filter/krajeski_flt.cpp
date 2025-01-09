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

KrajeskiMoog::KrajeskiMoog() {
}

KrajeskiMoog::KrajeskiMoog(float sampleRate) {
    for (int i = 0; i < 5; i++) {
        state[i] = delay[i] = 0;  
    }
    gComp = 1.0;
    SetDrive(0.0f);
    SetCutoff(1000.0f);
    SetResonance(0.1f);
}

KrajeskiMoog::~KrajeskiMoog() {
}

void KrajeskiMoog::SetCutoff(float c) 
{
    cutoff = c;
    wc = (float)(TWOPI * cutoff * DIV_SAMPLE_RATE);
    g = (float)(0.9892f * wc - 0.4342f * wc*wc + 0.1381f * wc*wc*wc - 0.0202f * wc*wc*wc*wc);
}