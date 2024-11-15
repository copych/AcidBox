#include "adsr.h"

void Adsr::init(float sample_rate, int blockSize) {
    sample_rate_  = sample_rate / blockSize;
    attackTarget_ = 1.0f + delta_;
    x_            = 0.0f;
    gate_         = false;
    mode_         = ADSR_SEG_IDLE;
    setSustainLevel(0.05f);
    setTimeMs(ADSR_SEG_ATTACK, 0.0f);
    setTimeMs(ADSR_SEG_DECAY, 1230.0f);
    setTimeMs(ADSR_SEG_RELEASE, 3.0f);
    setTimeMs(ADSR_SEG_FAST_RELEASE, 0.2f); // a few samples fade, trying to avoid clicks on polyphony overrun
    setTimeMs(ADSR_SEG_SEMI_FAST_RELEASE, 1.0f); // for exclusive note groups voice stealing
}


void Adsr::retrigger(eEnd_t hardness) {
  //DEBF("ADSR: %0.10f %0.10f %0.10f %0.10f\r\n", attackD0_, decayD0_, sus_level_, releaseD0_);
  gate_ = true;
  mode_ = ADSR_SEG_ATTACK;
  switch (hardness) {
    case END_NOW:
      x_ = 0.0f;
      D0_ = attackD0_;
      break;
    case END_FAST:
    case END_SEMI_FAST:
    case END_REGULAR:
    default:
      D0_ = attackD0_;
  }
}


void Adsr::end(eEnd_t hardness) {
  gate_ = false;
  switch (hardness) {
    case END_NOW:{
      target_ = attackTarget_;
      mode_ = ADSR_SEG_IDLE;
      D0_ = attackD0_;
      x_ = 0.f;
      break;
    }
    case END_FAST:{
      target_ = -delta_;
      mode_ = ADSR_SEG_FAST_RELEASE;
      D0_ = fastReleaseD0_;
      break;
    }
    case END_SEMI_FAST:{
      target_ = -delta_;
      mode_ = ADSR_SEG_SEMI_FAST_RELEASE;
      D0_ = semiFastReleaseD0_;
      break;
    }
    case END_REGULAR:
    default:{
      target_ = -delta_;
      mode_ = ADSR_SEG_RELEASE;
      D0_ = releaseD0_;
    }
  }
}


inline Adsr::eSegment_t Adsr::getCurrentSegment() {
  Adsr::eSegment_t ret = mode_;
  if (gate_ && (x_ == sus_level_)) {
    ret = ADSR_SEG_SUSTAIN;
  }
  return ret;
}


void Adsr::setTime(int seg, float time) {
  switch (seg) {
    case ADSR_SEG_ATTACK:
      {
        setTimeConstant(time, attackTime_, attackD0_, wholeRange_);
        break;
      }
    case ADSR_SEG_DECAY:
      {
        setTimeConstant(time, decayTime_, decayD0_, upperRange_);
      }
      break;
    case ADSR_SEG_RELEASE:
      {
        setTimeConstant(time, releaseTime_, releaseD0_, lowerRange_);
      }
      break;
    case ADSR_SEG_SEMI_FAST_RELEASE:
      {
        setTimeConstant(time, semiFastReleaseTime_, semiFastReleaseD0_, lowerRange_);
      }
      break;
    case ADSR_SEG_FAST_RELEASE:
      {
        setTimeConstant(time, fastReleaseTime_, fastReleaseD0_, lowerRange_);
      }
      break;
    default: return;
  }
}

void Adsr::setTimeConstant(float timeInS, float& time, float& coeff, float target) {
  if (timeInS != time) {
    time = timeInS;
    if (time > 0 ) {
      if (target > 0 ) {
        coeff = target * one_div(time * sample_rate_);
      } else {        
        coeff = 10.0f * one_div(time * sample_rate_);
      }
    } else
      coeff = 1.f;  // instant change
  }
}

void Adsr::setAttackTime(float timeInS) {
  setTimeConstant(timeInS, attackTime_, attackD0_, wholeRange_);
}

void Adsr::setDecayTime(float timeInS) {
  setTimeConstant(timeInS, decayTime_, decayD0_, upperRange_);
}

void Adsr::setReleaseTime(float timeInS) {
  setTimeConstant(timeInS, releaseTime_, releaseD0_, lowerRange_);
}

void Adsr::setFastReleaseTime(float timeInS) {
  setTimeConstant(timeInS, fastReleaseTime_, fastReleaseD0_, lowerRange_);
}

void Adsr::setSemiFastReleaseTime(float timeInS) {
  setTimeConstant(timeInS, semiFastReleaseTime_, semiFastReleaseD0_, lowerRange_);
}


void Adsr::setTimeMs(int seg, float time) {
  setTime(seg, time * 0.001f);
}

void Adsr::setAttackTimeMs(float timeInMs) {
  setTimeConstant(timeInMs*0.001f, attackTime_, attackD0_, wholeRange_);
}

void Adsr::setDecayTimeMs(float timeInMs) {
  setTimeConstant(timeInMs*0.001f, decayTime_, decayD0_, upperRange_);
}

void Adsr::setReleaseTimeMs(float timeInMs) {
  setTimeConstant(timeInMs*0.001f, releaseTime_, releaseD0_, lowerRange_);
}

void Adsr::setFastReleaseTimeMs(float timeInMs) {
  setTimeConstant(timeInMs*0.001f, fastReleaseTime_, fastReleaseD0_, lowerRange_);
}

void Adsr::setSemiFastReleaseTimeMs(float timeInMs) {
  setTimeConstant(timeInMs*0.001f, semiFastReleaseTime_, semiFastReleaseD0_, lowerRange_);
}


void Adsr::calcLogRanges() {
  float dd = one_div(delta_);
  wholeRange_ = logf((1.0f + delta_) * dd);
  upperRange_ = logf((1.0f - sus_level_ + delta_) * dd);
  lowerRange_ = logf((sus_level_ + delta_) * dd); 
 // DEBF("calc logs: whole %f, upper %f lower %f\r\n", wholeRange_, upperRange_, lowerRange_);
}

float Adsr::process() {
  float out = 0.0f;
  switch (mode_) {
    case ADSR_SEG_IDLE:
      out = 0.0f;
      break;
      
    case ADSR_SEG_ATTACK:
      x_ += (float)D0_ * ((float)attackTarget_ - (float)x_);
      out = x_;
      if (out > 1.0f) {
        mode_ = ADSR_SEG_DECAY;
        target_ = sus_level_;
        D0_ = decayD0_;
      }
      break;
      
    case ADSR_SEG_DECAY:
    case ADSR_SEG_RELEASE:
    case ADSR_SEG_FAST_RELEASE:
    case ADSR_SEG_SEMI_FAST_RELEASE:
      x_ += (float)D0_ * ((float)target_ - (float)x_);
      out = x_;
      if (out < 0) {
        mode_ = ADSR_SEG_IDLE;
        x_ = out = 0.0f;
        target_ = attackTarget_;
        D0_ = attackD0_;
      }
      break;
      
    default: 
      break;
  }
  return out;
}
