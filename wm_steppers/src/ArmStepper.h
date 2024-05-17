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

    byte _armheading = 0;
    byte _armcurrent = 0; //starts at 0 degrees
};


#endif