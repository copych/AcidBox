#include "overdrive.h"


void Overdrive::Init()
{
    SetDrive(0.0f);
}

float Overdrive::Process(float in)
{
    float pre = (float)(pre_gain_ * in * 2.0f);
    
    return (float)(fast_tanh(pre) * post_gain_ * compens_);
 //   return SoftClip(pre) * post_gain_;
}

void Overdrive::SetDrive(float drive)
{
    compens_ = fast_tanh(0.09f - 3.05f * drive) * 0.77f + 1.0f ;
    drive = 0.125f + (float)drive * (0.875f);
    //drive  = fclamp(drive, 0.f, 1.f);
    drive_ = 1.999f * (float)drive;

    const float drive_2    = (float)drive_ * drive_;
    const float pre_gain_a = (float)drive_ * 0.5f;
    const float pre_gain_b = (float)drive_2 * drive_2 * drive_ * 24.0f;
    pre_gain_              = (float)pre_gain_a + (float)(pre_gain_b - pre_gain_a) * drive_2;

    const float drive_squashed = drive_ * (2.0f - drive_);
    post_gain_ =  0.5f / fast_tanh((float)(0.33f + drive_squashed * (float)(pre_gain_ - 0.33f)));
#ifdef DEBUG_FX    
    DEBF("pre %0.4f post %0.4f drive %0.4f\r\n", pre_gain_, post_gain_, drive_);
#endif
}
