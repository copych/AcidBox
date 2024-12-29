#pragma once
#ifndef DELAY_H
#define DELAY_H
/*
 * This is a simple implementation of a delay line
 * - level adjustable
 * - feedback
 * - length adjustable
 *
 * Author: Marcel Licence
 * https://github.com/marcel-licence
 * 
 * - formed into a class
 * - buffer allocation 
 * by copych 2022
 * https://github.com/copych
 * 
 */

// max delay can be changed, but memory consumption will also change
#ifdef BOARD_HAS_PSRAM 
  #define MALLOC_CAP        (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
  #define MAX_DELAY         (SAMPLE_RATE)
#else
  #define MALLOC_CAP        (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
  #define MAX_DELAY         (SAMPLE_RATE/4)
#endif

class FxDelay {
	public:
		FxDelay() {}
		void Init(void);
		void Reset(void);
		inline void Process(float *signal_l, float *signal_r) __attribute__((always_inline));
		inline void SetFeedback(float value) __attribute__((always_inline));
		inline void SetLevel( float value ) __attribute__((always_inline));
		inline void SetLength( float value ) __attribute__((always_inline));

	private:
		//  module variables
		float *delayLine_l;
		float *delayLine_r;
		float delayToMix = 0.2f;
		float delayFeedback = 0.1f;
		 
		uint32_t delayLen = MAX_DELAY/4;
		uint32_t delayIn = 0;
		uint32_t delayOut = 0;
};
#endif