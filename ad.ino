#include "ad.h"

void AD_env::init(float sample_rate, int blockSize) {
    sample_rate_  = sample_rate / blockSize;
    attackTarget_ = 1.0f;
    attackTime_   = -1.f;
    decayTime_    = -1.f;
    sus_level_    = 1.0f;
    x_            = 0.0f;
    gate_         = false;
    mode_         = AD_SEG_IDLE;

    setTimeMs(AD_SEG_ATTACK, 3.0f);
    setTimeMs(AD_SEG_DECAY, 1000.0f);
}


void AD_env::retrigger(bool hard) {
  gate_ = true;
  mode_ = AD_SEG_ATTACK;
  if (hard) {
      x_ = 0.0f;
      D0_ = attackD0_;
      peakLevel_ = 1.0f;
      attackTarget_ = peakLevel_ + 0.1f;
  } else {
      D0_ = attackD0_;
      peakLevel_ = 1.0f + (fast_shape( x_ ));
      attackTarget_ = peakLevel_ + 0.1f ;
  }
}


void AD_env::end(bool hard) {
  gate_ = false;
  target_ = -0.1f;
  if (hard) { 
    mode_ = AD_SEG_IDLE;
    D0_ = attackD0_;
    x_ = 0.f; 
  } else {
    mode_ = AD_SEG_DECAY;
    D0_ = decayD0_;
  }
}


inline AD_env::eSegment_t AD_env::getCurrentSegment() {
  AD_env::eSegment_t ret = mode_;
  return ret;
}


void AD_env::setTime(int seg, float time) {
  switch (seg) {
    case AD_SEG_ATTACK:
      {
        setAttackTime(time);  
        break;
      }
    case AD_SEG_DECAY:
      {
        setDecayTime(time);
      }
      break;
    default: return;
  }
}

void AD_env::setAttackTime(float timeInS) {
  if (timeInS != attackTime_) {
    attackTime_ = timeInS;
    if (timeInS > 0) {
      attackD0_ = 2.4f /(timeInS * sample_rate_);
    } else
      attackD0_ = 1.f;  // instant change
  }
}


void AD_env::setDecayTime(float timeInS) {  
  if (timeInS != decayTime_) {
    decayTime_ = timeInS;
    if (decayTime_ > 0) { 
      decayD0_ = 2.4f / (timeInS * sample_rate_);
    } else
      decayD0_ = 1.f;  // instant change
  }
}


void AD_env::setTimeMs(int seg, float timeInMs) {setTime(seg, 0.001f*timeInMs); }

void AD_env::setAttackTimeMs(float timeInMs) {setAttackTime(0.001f*timeInMs);}

void AD_env::setDecayTimeMs(float timeInMs) {setDecayTime(0.001f*timeInMs);}

float AD_env::process() {
  float out;
  switch (mode_) {
    case AD_SEG_IDLE:
      out = 0.0f;
      break;
    case AD_SEG_ATTACK:
      x_ += (float)D0_ * ((float)attackTarget_ - (float)x_);
      out = x_;
      if (out > peakLevel_ ) {
        mode_ = AD_SEG_DECAY;
        target_ = -0.1f;
        D0_ = decayD0_;
      }
      break;
    case AD_SEG_DECAY:
      x_ += (float)D0_ * ((float)target_ - (float)x_ );
      out = x_;
      if (out < 0.0f) {
        mode_ = AD_SEG_IDLE;
        x_ = out = 0.0f;
        target_ = 1.0f;
        D0_ = attackD0_;
      }
      break;
    default:
      out = 0.0f;
      break;
  }
  return out;
}
