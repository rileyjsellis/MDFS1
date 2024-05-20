#ifndef ArmSteppers_h
#define ArmSteppers_h
#include "Arduino.h"

class ArmSteppers {
  public: 
    ArmSteppers(byte allStepperPins[2][2]);
    void moveTo(int pod_pos[2]);
    void moving();

  private:
    byte _pinMotion[2];
    byte _pinDirection[2];

    const int _kNodeXToPlatformX = 130;

    const int _kClawOverPodY = 90; //these two cancel out to be 40, create new const.
    const int _kBaseNodeY = 50;
    const int _kAddY = _kClawOverPodY - _kBaseNodeY;

    const int _kLowerArmLength = 300;
    const float _kUpperArmLength = 250.24;

    int _pod_pos[2]; //x and y inputs
    int _hyp_to_pos;
    float _theta;



    byte _armheading[2] = {0,0};
    byte _armcurrent[2] = {0,0}; //starts at 0 degrees, check solidworks calculations.
};

#endif