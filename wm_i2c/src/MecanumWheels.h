#ifndef MecanumWheels_h
#define MecanumWheels_h
#include "Arduino.h"

class MecanumWheels {
  public: 
    MecanumWheels(byte allPins[3][4]);
    void goForward();
    void goRightBackwards();
    void goRight();
    void goLeft();
    void spin(int direction);
    void rest();
  private:
    byte _enables[4]; //needed to define max limit
    byte _forwards[4];
    byte _backwards[4];
    void wheelsOn(bool answer);
};

#endif