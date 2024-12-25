#ifndef DEBUG_TIMING_H
#define DEBUG_TIMING_H

struct Debug {
    static volatile inline uint32_t s1t, s2t, drt, fxt, s1T, s2T, drT, fxT, art, arT, c0t, c0T, c1t, c1T; // debug timing: if we use less vars, compiler optimizes them
};

#define RECORD_TIME(TIMER_VAR, FUNCTION_CALL, OUTPUT_VAR) TIMER_VAR = micros(); FUNCTION_CALL; OUTPUT_VAR = micros() - TIMER_VAR;


#endif