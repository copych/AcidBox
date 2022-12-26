/*
 * This is a simple implementation of a delay line
 * - level adjustable
 * - feedback
 * - length adjustable
 *
 * Author: Marcel Licence
 */

#define MAX_DELAY SAMPLE_RATE // 1 second

class FxDelay {
	public:
		FxDelay() {}
   
		// max delay can be changed but changes also the PSRAM consumption
		void Init( void ){
			delayLine_l = (float *)ps_malloc(sizeof(float) * MAX_DELAY);
			if( delayLine_l == NULL){
				DEBF("No more PSRAM memory!\n");
			}
			delayLine_r = (float *)ps_malloc(sizeof(float) * MAX_DELAY);
			if( delayLine_r == NULL ){
				DEBF("No more PSRAM memory!\n");
			}
			Reset();
		};

		void Reset( void ){
			for( int i = 0; i < MAX_DELAY; i++ ){
				delayLine_l[i] = 0;
				delayLine_r[i] = 0;
			}
			delayLen = 60.0f / 130.0f * 1.5f * (float)SAMPLE_RATE;
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
