#include "mixer.h"

Mixer::Mixer(SynthVoice *synth1, SynthVoice *synth2, Sampler *drums) {
    _synth1 = synth1;
    _synth2 = synth2;
    _drums = drums;
}

void IRAM_ATTR Mixer::mix() {
    #ifdef DEBUG_MASTER_OUT
    static float meter = 0.0f;
    #endif
    static float synth1_out_l, synth1_out_r, synth2_out_l, synth2_out_r, drums_out_l, drums_out_r;
    static float dly_l, dly_r, rvb_l, rvb_r;
    static float mono_mix;
        dly_k1 = _synth1->_sendDelay;
        dly_k2 = _synth2->_sendDelay;
        dly_k3 = _drums->_sendDelay;
    #ifndef NO_PSRAM 
        rvb_k1 = _synth1->_sendReverb;
        rvb_k2 = _synth2->_sendReverb;
        rvb_k3 = _drums->_sendReverb;
    #endif
        for (int i=0; i < DMA_BUF_LEN; i++) { 
        drums_out_l = _drums->drums_buf_l[current_out_buf][i];
        drums_out_r = _drums->drums_buf_r[current_out_buf][i];

        synth1_out_l = _synth1->GetPan() * _synth1->synth_buf[current_out_buf][i];
        synth1_out_r = (1.0f - _synth1->GetPan()) * _synth1->synth_buf[current_out_buf][i];
        synth2_out_l = _synth2->GetPan() * _synth2->synth_buf[current_out_buf][i];
        synth2_out_r = (1.0f - _synth2->GetPan()) * _synth2->synth_buf[current_out_buf][i];

        dly_l = dly_k1 * synth1_out_l + dly_k2 * synth2_out_l + dly_k3 * drums_out_l; // delay bus
        dly_r = dly_k1 * synth1_out_r + dly_k2 * synth2_out_r + dly_k3 * drums_out_r;
        Delay.Process( &dly_l, &dly_r );
    #ifndef NO_PSRAM
        rvb_l = rvb_k1 * synth1_out_l + rvb_k2 * synth2_out_l + rvb_k3 * drums_out_l; // reverb bus
        rvb_r = rvb_k1 * synth1_out_r + rvb_k2 * synth2_out_r + rvb_k3 * drums_out_r;
        Reverb.Process( &rvb_l, &rvb_r );

        mix_buf_l[current_out_buf][i] = (synth1_out_l + synth2_out_l + drums_out_l + dly_l + rvb_l);
        mix_buf_r[current_out_buf][i] = (synth1_out_r + synth2_out_r + drums_out_r + dly_r + rvb_r);
    #else
        mix_buf_l[current_out_buf][i] = (synth1_out_l + synth2_out_l + drums_out_l + dly_l);
        mix_buf_r[current_out_buf][i] = (synth1_out_r + synth2_out_r + drums_out_r + dly_r);
    #endif
        mono_mix = 0.5f * (mix_buf_l[current_out_buf][i] + mix_buf_r[current_out_buf][i]);
    //    Comp.Process(mono_mix);     // calculate gain based on a mono mix

        Comp.Process(drums_out_l*0.25f);  // calc compressor gain, side-chain driven by drums


        mix_buf_l[current_out_buf][i] = (Comp.Apply( 0.25f * mix_buf_l[current_out_buf][i]));
        mix_buf_r[current_out_buf][i] = (Comp.Apply( 0.25f * mix_buf_r[current_out_buf][i]));

        
    #ifdef DEBUG_MASTER_OUT
        if ( i % 16 == 0) meter = meter * 0.95f + fabs( mono_mix); 
    #endif
    //    mix_buf_l[current_out_buf][i] = General::fclamp(mix_buf_l[current_out_buf][i] , -1.0f, 1.0f); // clipper
    //    mix_buf_r[current_out_buf][i] = General::fclamp(mix_buf_r[current_out_buf][i] , -1.0f, 1.0f);
        mix_buf_l[current_out_buf][i] = General::fast_shape( mix_buf_l[current_out_buf][i]); // soft limitter/saturator
        mix_buf_r[current_out_buf][i] = General::fast_shape( mix_buf_r[current_out_buf][i]);
    }
    #ifdef DEBUG_MASTER_OUT
    meter *= 0.95f;
    meter += fabs(mono_mix); 
    DEBF("out= %0.5f\r\n", meter);
    #endif
}