#include "fx_delay.h"

void FxDelay::Init(void) {
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

void FxDelay::Reset(void){
    //float bpm = 120;
    for (int i = 0; i < MAX_DELAY; i++){
        delayLine_l[i] = 0;
        delayLine_r[i] = 0;
    }
    delayLen = 60.0f / bpm * 1.5f * (float)min(SAMPLE_RATE,MAX_DELAY);
    delayToMix = 1.0f;
    delayFeedback = 0.2f;
}

void FxDelay::Process(float *signal_l, float *signal_r) {
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
}

void FxDelay::SetFeedback(float value) {
    delayFeedback = value;
#ifdef DEBUG_FX
    DEBF("delay feedback: %0.3f\n", value);
#endif
}

void FxDelay::SetLevel(float value) {
    delayToMix = value;
#ifdef DEBUG_FX
    DEBF("delay level: %0.3f\n", value);
#endif
}

void FxDelay::SetLength(float value) {
    delayLen = (uint32_t)(((float)MAX_DELAY - 1.0f) * value);
#ifdef DEBUG_FX
    DEBF("delay length: %0.3fms\n", delayLen * (1000.0f / ((float)SAMPLE_RATE)));
#endif
}