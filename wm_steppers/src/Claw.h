#ifndef Claw_h
#define Claw_h
#include "Arduino.h"

class Claw {
  public: 
    Claw(byte pin);
    void open();
    void close();
  private:
    byte _pin;
};


#endif