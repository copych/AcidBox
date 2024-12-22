#ifndef GENERAL_H
#define GENERAL_H

class General {
  public:
    static float fclamp(float in, float min, float max) __attribute__((noinline));
};

#endif