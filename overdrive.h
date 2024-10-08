#pragma once


class Overdrive
{
  public:
    Overdrive() {}
    ~Overdrive() {}
 
    void Init();
    float Process(float in);
    void SetDrive(float drive);

  private:
    float _drive;
    float _pre_gain;
    float _post_gain;
    BiquadFilter midBoost1;
    BiquadFilter midBoost2;
}; 


void Overdrive::Init()
{
    SetDrive(0.0f);

    midBoost1.setMode(BiquadFilter::BANDPASS);
    midBoost1.setFrequency(1800.0f);
    midBoost1.setBandwidth(4.0f);
    midBoost2.setMode(BiquadFilter::BANDPASS);
    midBoost2.setFrequency(1500.0f);
    midBoost2.setBandwidth(4.0f);
}

float Overdrive::Process(float in)
{
   // in = midBoost1.getSample(in)*4.0f;
    float pre = (float)(_pre_gain * in * 2.0f);
    float out = (float)(fast_shape(pre) * _post_gain) ; 
 //   out = midBoost2.getSample(out)*4.0f;
    return out;
}

void Overdrive::SetDrive(float drive)
{
 //   compens_ = fast_shape(0.09f - 3.05f * drive) * 0.77f + 1.0f ;
    //midBoost1.setBandwidth(20.0f - 15.0f * drive);
    //midBoost2.setBandwidth(20.0f - 15.0f * drive);
    drive = 0.125f + (float)drive * (0.875f);
    //drive  = fclamp(drive, 0.f, 1.f);
    _drive = 1.999f * (float)drive;

     float drive2    = (float)_drive * _drive;
     float pregain_a = (float)_drive * 0.5f;
     float pregain_b = (float)drive2 * drive2 * _drive * 24.0f ;
    _pre_gain              = (float)pregain_a + (float)(pregain_b - pregain_a) * drive2;

     float _drivesquashed = _drive * (2.0f - _drive);
     _post_gain =   one_div( fast_shape((float)(0.33f + _drivesquashed * (float)(_pre_gain - 0.33f))));
#ifdef DEBUG_FX    
    DEBF("pre %0.4f post %0.4f drive %0.4f\r\n", _pre_gain, _post_gain, _drive);
#endif
}
