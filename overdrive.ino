#include "overdrive.h"


void Overdrive::Init()
{
    SetDrive(.5f);
}

float Overdrive::Process(float in)
{
    float pre = pre_gain_ * in * 2.0;
    
    return fast_tanh(pre) * post_gain_;
 //   return SoftClip(pre) * post_gain_;
}

void Overdrive::SetDrive(float drive)
{
    drive  = fclamp(drive, 0.f, 1.f);
    drive_ = 1.999f * drive;

    const float drive_2    = drive_ * drive_;
    const float pre_gain_a = drive_ * 0.5f;
    const float pre_gain_b = drive_2 * drive_2 * drive_ * 24.0f;
    pre_gain_              = pre_gain_a + (pre_gain_b - pre_gain_a) * drive_2;

    const float drive_squashed = drive_ * (2.0f - drive_);
    post_gain_ = 0.5f / fast_tanh(0.33f + drive_squashed * (pre_gain_ - 0.33f));
    DEBF("pre %0.4f post %0.4f drive %0.4f\r\n", pre_gain_, post_gain_, drive_);
}
