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
    // delayLen = 60.0f / bpm * 1.5f * (float)acidbox_min(SAMPLE_RATE,MAX_DELAY);
    // delayToMix = 1.0f;
    // delayFeedback = 0.2f;
    delayLen = 0.0f;
    delayToMix = 0.0f;
    delayFeedback = 0.0f;    
}