#ifndef Claw_h
#define Claw_h
#include "Arduino.h"

// Code Overview
// Claw is a much simpler class than Stepper, relying on myServo.
// Here there are only two functions, open and closed.

// As the code is ready before the claw. These positions will 
// come through testing, so making use of constants is the most 
// reasonable decision.

class Claw {
  public: 
    Claw(byte pin);
    void open();
    void close();
  private:
    byte _pin;
};


#endif