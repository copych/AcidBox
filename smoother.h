#pragma once
#ifndef SMOOTHER_H
#define SMOOTHER_H
/*
  Median sliding window by Alex Gyver https://alexgyver.ru/lessons/filters/

*/
#define NUM_READ 5
class Smoother {
  public:

    Smoother() {}
    ~Smoother() {}
    inline float Process(float newVal) {
      float buff;
      static uint8_t count = 0;
      buffer[count] = newVal;
      if ((count < NUM_READ - 1) and (buffer[count] > buffer[count + 1])) {
        for (int i = count; i < NUM_READ - 1; i++) {
          if (buffer[i] > buffer[i + 1]) {
            buff = buffer[i];
            buffer[i] = buffer[i + 1];
            buffer[i + 1] = buff;
          }
        }
      } else {
        if ((count > 0) and (buffer[count - 1] > buffer[count])) {
          for (int i = count; i > 0; i--) {
            if (buffer[i] < buffer[i - 1]) {
              buff = buffer[i];
              buffer[i] = buffer[i - 1];
              buffer[i - 1] = buff;
            }
          }
        }
      }
      if (++count >= NUM_READ) count = 0;
      return buffer[(int)(NUM_READ * 0.5)];
    }


  private:
    float buffer[NUM_READ];
};

#endif
