#pragma once
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
		void Init( void ){
    // heap_caps_print_heap_info(MALLOC_CAP_8BIT);
      delayLine_l = (float *)heap_caps_calloc(1, sizeof(float) * MAX_DELAY, MALLOC_CAP);
      if( delayLine_l == NULL){
        DEBUG("DELAY: No more memory for delay L !");
      } else {
        DEBUG("DELAY: Memory allocated for delay L");
      }
   // heap_caps_print_heap_info(MALLOC_CAP_8BIT);
      delayLine_r = (float *)heap_caps_calloc(1, sizeof(float) * MAX_DELAY, MALLOC_CAP);
      if( delayLine_r == NULL ){
        DEBF("DELAY: No more memory for delay R !");
      } else {
        DEBUG("DELAY: Memory allocated for delay R");
      }
      Reset();
		}

		void Reset( void ){
      //float bpm = 120;
			for (int i = 0; i < MAX_DELAY; i++ ){
				delayLine_l[i] = 0;
				delayLine_r[i] = 0;
			}
			delayLen = 60.0f / bpm * 1.5f * (float)min(SAMPLE_RATE,MAX_DELAY);
			delayToMix = 1.0f;
			delayFeedback = 0.2f;
		};

		inline void Process( float *signal_l, float *signal_r ){

		  delayLine_l[delayIn] = *signal_l;
		  delayLine_r[delayIn] = *signal_r;
		  
		  delayOut = delayIn + (1 + MAX_DELAY - delayLen);
		  
		  if( delayOut >= MAX_DELAY ){
			  delayOut -= MAX_DELAY;
		  }

		  *signal_l += delayLine_l[delayOut] * delayToMix;
		  *signal_r += delayLine_r[delayOut] * delayToMix;
		  
		  delayLine_l[delayIn] += delayLine_l[delayOut] * delayFeedback;
		  delayLine_r[delayIn] += delayLine_r[delayOut] * delayFeedback;
		  
		  delayIn ++;
		  
		  if( delayIn >= MAX_DELAY ){
			  delayIn = 0;
		  }
		};

		inline void SetFeedback( float value){
			delayFeedback = value;
#ifdef DEBUG_FX
			DEBF("delay feedback: %0.3f\n", value);
#endif
		};

		inline void SetLevel( float value ){
			delayToMix = value;
#ifdef DEBUG_FX
			DEBF("delay level: %0.3f\n", value);
#endif
		};

		inline void SetLength( float value ){
			delayLen = (uint32_t)(((float)MAX_DELAY - 1.0f) * value);
#ifdef DEBUG_FX
			DEBF("delay length: %0.3fms\n", delayLen * (1000.0f / ((float)SAMPLE_RATE)));
#endif
		};

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
