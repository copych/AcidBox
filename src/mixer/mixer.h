#ifndef MIXER_H
#define MIXER_H
#include "../fx/fx_delay.h"
#include "../fx/fx_reverb.h"
#include "../synth/synthvoice.h"
#include "../sampler/sampler.h"
#include "../fx/compressor.h"

class Mixer {
  public:
    Mixer(SynthVoice *synth1, SynthVoice *synth2, Sampler *drums, FxDelay *fxDelay, Compressor *compressor, FxReverb *reverb, volatile int *current_out_buf);
    Mixer(SynthVoice *synth1, SynthVoice *synth2, Sampler *drums, FxDelay *fxDelay, Compressor *compressor, volatile int *current_out_buf);
    void mix() __attribute__((noinline));
    float WORD_ALIGNED_ATTR mix_buf_l[2][DMA_BUF_LEN] = {0.0f};
    float WORD_ALIGNED_ATTR mix_buf_r[2][DMA_BUF_LEN] = {0.0f};

  private:
    volatile int *_current_out_buf;
    SynthVoice *_synth1;
    SynthVoice *_synth2;
    Sampler *_drums;
    FxDelay *_delay;
    Compressor *_compressor;
    FxReverb *_reverb;
    volatile float rvb_k1, rvb_k2, rvb_k3;
    volatile float dly_k1, dly_k2, dly_k3;
};
#endif