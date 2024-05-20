#ifndef Claw_h
#define Claw_h
#include "Arduino.h"
//my servo library to make use of?

class Claw {
  public: 
    Claw(byte pin);
    void open();
    void close();
  private:
    byte _pin;
};


#endif