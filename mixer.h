#ifndef MIXER_H
#define MIXER_H
#include "synthvoice.h"
#include "sampler.h"
#include "fx_delay.h"
#include "compressor.h"
#include "fx_reverb.h"

class Mixer {
  public:
    Mixer(SynthVoice *synth1, SynthVoice *synth2, Sampler *drums, FxDelay *fxDelay, Compressor *compressor, FxReverb *reverb);
    void mix() __attribute__((noinline));
    float WORD_ALIGNED_ATTR mix_buf_l[2][DMA_BUF_LEN] = {0.0f};
    float WORD_ALIGNED_ATTR mix_buf_r[2][DMA_BUF_LEN] = {0.0f};

  private:
    SynthVoice *_synth1;
    SynthVoice *_synth2;
    Sampler *_drums;
    FxDelay *_delay;
    Compressor *_compressor;
    FxReverb *_reverb;
};
#endif