#ifndef ArmStepper_h
#define ArmStepper_h
#include "Arduino.h"

class ArmStepper {
  public: 
    ArmStepper(byte pinMotion, byte pinDirection);
    void moveTo(int steps);
    void moving();
  private:
    byte _pinMotion;
    byte _pinDirection;

    byte _armresting = 200; //use steps rather than angles, convert as they come along.
};


#endif