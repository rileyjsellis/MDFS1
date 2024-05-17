#ifndef ArmSteppers_h
#define ArmSteppers_h
#include "Arduino.h"

class ArmSteppers {
  public: 
    ArmSteppers(byte allStepperPins[2][2]);
    void moveTo(int topPosition, int lowerPosition);
    void moving();

  private:
    byte _pinMotion[2];
    byte _pinDirection[2];

    byte _armheading[2] = {0,0};
    byte _armcurrent[2] = {0,0}; //starts at 0 degrees, check solidworks calculations.
};

#endif