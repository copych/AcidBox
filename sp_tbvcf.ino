/*
 * TBVCF
 *
 * This code has been extracted from the Csound opcode "tbvcf".
 * It has been modified to work as a Soundpipe module.
 *
 * Original Author(s): Hans Mikelson
 * Year: 2000
 * Location: Opcodes/biquad.c
 *
 */


#include <math.h>
#include "sp_tbvcf.h"
//#include <stdint.h>
//#include <stdlib.h>
#define ROOT2 (1.4142135623730950488)

inline void SP_TBVCF::Init(float smp_rate)
{
  	_sr = smp_rate;   
  	_fco = 1500.0;
    _res = 0.8;
    _dist = 2.0;
    _asym = 0.5;

    _onedsr = 1.0 / _sr;

    _y = _y1 = _y2 = 0.0;

    _fcocod = (int)_fco;
    _rezcod = (int)_res;
}

inline float SP_TBVCF::Process( float x )
{ 
    float fco, res, dist, asym, out;
    float y = _y, y1 = _y1, y2 = _y2;
    // The initialisations are fake to fool compiler warnings 
    float ih, fdbk, d, ad;
    float fc=0.0, fco1=0.0, q=0.0, q1=0.0;

    ih  = 0.001; // ih is the incremental factor 

    fco = _fco;
    res = _res;
    dist = _dist;
    asym = _asym;

 /* Try to decouple the variables */
    if ((_rezcod==0) && (_fcocod==0)) { /* Calc once only */
        q1   = res/(1.0 + sqrt(dist));
        fco1 = pow(fco*260.0/(1.0+q1*0.5),0.58);
        q    = q1*fco1*fco1*0.0005;
        fc   = fco1*_onedsr*(_sr*0.125f);
    }
    if ((_rezcod!=0) || (_fcocod!=0)) {
        q1  = res/(1.0 + sqrt(dist));
        fco1 = pow(fco*260.0/(1.0+q1*0.5),0.58);
        q  = q1*fco1*fco1*0.0005;
        fc  = fco1*_onedsr*(_sr*0.125f);
    } 
    fdbk = q*y/(1.0 + exp(-3.0*y)*asym);
    y1  = y1 + ih*((x - y1)*fc - fdbk);
    d  = -0.1*y*20.0;
    ad  = (d*d*d + y2)*100.0*dist;
    y2  = y2 + ih*((y1 - y2)*fc + ad);
    y  = y + ih*((y2 - y)*fc);
    out = (y*fc*0.001f*(1.0 + q1)*3.2);

    _y = y; _y1 = y1; _y2 = y2;
    return out;
}
