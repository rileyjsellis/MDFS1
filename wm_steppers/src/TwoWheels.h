#ifndef TwoWheels_h
#define TwoWheels_h
#include "Arduino.h"

class TwoWheels {
 public:
  TwoWheels(byte pin[3][2]);
  void setSpeed();
  void forwards();
  void backwards();
  void spinLeft();
  void spinRight();

  void reverseRight(int turningRadius);
  void turnLeft(int turningRadius);

  void stop();

 private:
  byte _forwards[2];
  byte _backwards[2];
  byte _enables[2];

  const int topSpeed = 255;

  // abstracted private functions to reduce code.
  void leftWheelForwards(bool isForward, int speed);
  void rightWheelForwards(bool isForward, int speed);
  void wheelForwards(bool isLeft, bool isForward, int speed);
};

#endif