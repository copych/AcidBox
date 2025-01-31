#pragma once

#ifndef KRAJESKI_LADDER_H
#define KRAJESKI_LADDER_H

#include "../../config.h"
#include "../general/general.h"

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
*/

class KrajeskiMoog 
{
		
	public:
	KrajeskiMoog();
	KrajeskiMoog(float sampleRate);
	~KrajeskiMoog();
	inline float Process(float sample) __attribute__((always_inline));
	inline void Init(float samplerate) __attribute__((always_inline));
	inline void SetResonance(float r) __attribute__((always_inline));
	inline void SetDrive(float dr) __attribute__((always_inline));
	void SetCutoff(float c);

	private:
		float state[5];
		float delay[5];
		float wc; // The angular frequency of the cutoff.
		float g; // A derived parameter for the cutoff frequency
		float gRes; // A similar derived parameter for resonance.
		float gComp; // Compensation factor.
		float drive; // A parameter that controls intensity of nonlinearities.
		float sampleRate, cutoff, resonance;
};

inline void KrajeskiMoog::Init(float samplerate) {
	gComp = 1.0;
	sampleRate = samplerate;

	SetDrive(0.0f);
	SetCutoff(1000.0f);
	SetResonance(0.1f);
}

inline void KrajeskiMoog::SetResonance(float r) 
{
	resonance = r*1.3f;
	gRes = (float)( resonance * (1.0029f + 0.0526f * wc - 0.926f * wc*wc + 0.0218f * wc*wc*wc));
}

inline void KrajeskiMoog::SetDrive(float dr) {
	drive = dr * 2.0f + 1.0f;  
}

inline float KrajeskiMoog::Process(float sample) 
{
	state[0] = General::fast_shape(drive * (sample - 4.0f * gRes * (state[4] - gComp * sample)));
	
	for(int i = 0; i < 4; i++)
	{
	state[i+1] = General::fclamp(g * ( 0.2307692f * state[i] + 0.7692307f * delay[i] - state[i + 1]) + state[i + 1], -1e30, 1e30 );
	delay[i] = state[i];

	}
	return state[4];
}

#endif
