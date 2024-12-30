/*
 * reverb stuff 
 * Author: Marcel Licence
 * https://github.com/marcel-licence
 *
 * This is a port of the public code from YetAnotherElectronicsChannel
 * Based on main.c
 *
 * Changes:
 * - optimized for buffer processing
 * - added interface to set the level
 * 
 * src: https://github.com/YetAnotherElectronicsChannel/STM32_DSP_Reverb/blob/master/code/Src/main.c
 * The explanation of the original module can be found here: https://youtu.be/nRLXNmLmHqM
 *
 *
 * - formed into a class 
 * - buffer allocation
 * - bug fixes
 * by copych 2022-2023
 * https://github.com/copych
 * 
 */ 
#pragma once
#ifndef REVERB_H
#define REVERB_H

#ifdef BOARD_HAS_PSRAM 
  #define REV_MULTIPLIER 1.797f
  #define MALLOC_CAP        (MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT)
#else
  #define REV_MULTIPLIER 0.349f
  #define MALLOC_CAP        (MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT)
#endif

#define COMB_BUF_LEN_0    (int)( 3603.0f * REV_MULTIPLIER)
#define COMB_BUF_LEN_1    (int)( 3113.0f * REV_MULTIPLIER)
#define COMB_BUF_LEN_2    (int)( 4043.0f * REV_MULTIPLIER)
#define COMB_BUF_LEN_3    (int)( 4492.0f * REV_MULTIPLIER)
#define ALLPASS_BUF_LEN_0 (int)(  503.0f * REV_MULTIPLIER)
#define ALLPASS_BUF_LEN_1 (int)(  167.0f * REV_MULTIPLIER)
#define ALLPASS_BUF_LEN_2 (int)(   47.0f * REV_MULTIPLIER)

//rev_time 0.0 <-> 1.0
//rev_delay 0.0 <-> 1.0

class FxReverb {
  public:
    FxReverb() {}
    void Process(float *signal_l, float *signal_r) __attribute__((always_inline));
    void Init() __attribute__((always_inline));
    void SetTime(float value) __attribute__((always_inline));
    void SetLevel(float value) __attribute__((always_inline));
    
  private:
    float rev_time = 0.5f;
    float rev_level = 0.5f;
    float* combBuf0     = nullptr;
    float* combBuf1     = nullptr;
    float* combBuf2     = nullptr;
    float* combBuf3     = nullptr;
    float* allPassBuf0  = nullptr;
    float* allPassBuf1  = nullptr;
    float* allPassBuf2  = nullptr;    
  
    //define pointer limits = delay time
    int cf0_lim, cf1_lim, cf2_lim, cf3_lim, ap0_lim, ap1_lim, ap2_lim;

    int cf0_p = 0;
    float cf0_g = 0.805f;      
    int cf1_p = 0;
    float cf1_g = 0.827f;
    int cf2_p = 0;
    float cf2_g = 0.783f;
    int cf3_p = 0;
    float cf3_g = 0.764f;
    int ap0_p = 0;
    float ap0_g = 0.7f;
    int ap1_p = 0;
    float ap1_g = 0.7f;
    int ap2_p = 0;
    float ap2_g = 0.7f;
    
    float Do_Comb0(float inSample) __attribute__((always_inline));
    float Do_Comb1(float inSample) __attribute__((always_inline));
    float Do_Comb2(float inSample) __attribute__((always_inline));
    float Do_Comb3(float inSample) __attribute__((always_inline));
    float Do_Allpass0(float inSample) __attribute__((always_inline));
    float Do_Allpass1(float inSample) __attribute__((always_inline));
    float Do_Allpass2(float inSample) __attribute__((always_inline));
};
#endif