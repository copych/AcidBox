/*
 * This is a simple implementation of a delay line
 * - level adjustable
 * - feedback
 * - length adjustable
 *
 * Author: Marcel Licence
 */


// max delay can be changed but changes also the memory consumption

#define MAX_DELAY  44100
//  module variables
float *delayLine_l;
float *delayLine_r;
float delayToMix = 0.2f;
float delayFeedback = 0.1f;
 
uint32_t delayLen = 10000;
uint32_t delayIn  = 0;
uint32_t delayOut = 0;

void Delay_Init( void ){
    delayLine_l = (float *)ps_malloc(sizeof(float) * MAX_DELAY);
    if( delayLine_l == NULL){
        DEBF("No more PSRAM memory!\n");
    }
    delayLine_r = (float *)ps_malloc(sizeof(float) * MAX_DELAY);
    if( delayLine_r == NULL ){
        DEBF("No more PSRAM memory!\n");
    }
    Delay_Reset();
}

void Delay_Reset( void ){
    for( int i = 0; i < MAX_DELAY; i++ ){
        delayLine_l[i] = 0;
        delayLine_r[i] = 0;
    }
}
/*
void Delay_Sync_Values(){
  if( delayToMix_old != delayToMix_midi ){
    delayToMix_old = delayToMix_midi;
    delayToMix     = delayToMix_midi * MIDI_NORM;
  }
  if( delayFeedback_old != delayFeedback_midi ){
    delayFeedback_old = delayFeedback_midi;
    delayFeedback     = delayFeedback_midi * MIDI_NORM / 2
    
    ;
  }
  if( delayLen_old != delayLen_midi ){
    delayLen_old = delayLen_midi;
    delayLen  = 16 + delayLen_midi *40; // 85; // DelayLen darf nicht 0 sein!
  }  
}

*/

void Delay_Process( float *signal_l, float *signal_r ){

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

void Delay_SetFeedback(uint8_t unused, float value){
    delayFeedback = value;
    DEBF("delay feedback: %0.3f\n", value);
}

void Delay_SetLevel(uint8_t unused, float value ){
    delayToMix = value;
    DEBF("delay level: %0.3f\n", value);
}

void Delay_SetLength(uint8_t unused, float value ){
    delayLen = (uint32_t)(((float)MAX_DELAY - 1.0f) * value);
    DEBF("delay length: %0.3fms\n", delayLen * (1000.0f / ((float)SAMPLE_RATE)));
}
