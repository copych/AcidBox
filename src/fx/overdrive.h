#pragma once
#ifndef OVERDRIVE_H
#define OVERDRIVE_H
#include "general.h"
#include "../filter/rosic_BiquadFilter.h"

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

#endif