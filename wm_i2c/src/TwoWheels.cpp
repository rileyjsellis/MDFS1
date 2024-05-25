#include "TwoWheels.h"

#include "Arduino.h"

TwoWheels::TwoWheels(byte allPins[3][2]) {  // enables all pins as OUTPUT
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 2; j++) {
      pinMode(allPins[i][j], OUTPUT);
      switch (i) {
        case 0:
          _enables[j] = allPins[i][j];
          break;
        case 1:
          _forwards[j] = allPins[i][j];
          break;
        case 2:
          _backwards[j] = allPins[i][j];
          break;
      }
    }
  }
}

// Private functions to abstract away pin setup for directions and speed of each
// wheel.
void TwoWheels::wheelForwards(bool isLeft, bool isForward, int speed) {
  digitalWrite(_enables[isLeft], speed);
  digitalWrite(_forwards[isLeft], isForward);
  digitalWrite(_backwards[isLeft], isForward);
}

void TwoWheels::leftWheelForwards(bool isForward, int speed) {
  wheelForwards(true, isForward, speed);
}

void TwoWheels::rightWheelForwards(bool isForward, int speed) {
  wheelForwards(false, isForward, speed);
}

// Public functions for user to call in state machine.
void TwoWheels::forwards() {
  leftWheelForwards(true, topSpeed);
  rightWheelForwards(true, topSpeed);
}

void TwoWheels::backwards() {
  leftWheelForwards(false, topSpeed);
  rightWheelForwards(false, topSpeed);
}

void TwoWheels::spinLeft() {
  leftWheelForwards(false, topSpeed);
  rightWheelForwards(true, topSpeed);
}

void TwoWheels::spinRight() {
  leftWheelForwards(true, topSpeed);
  rightWheelForwards(false, topSpeed);
}

void TwoWheels::turnLeft(int turningRadius) {
  leftWheelForwards(true, (topSpeed * (10 - turningRadius)/10));
  rightWheelForwards(true, topSpeed);
}

void TwoWheels::turnRight(int turningRadius) {
  leftWheelForwards(true, topSpeed);
  rightWheelForwards(true, (topSpeed * (10 - turningRadius)/10));
}

void TwoWheels::stop() {
  for (int i = 0; i < 2; i++) {
    digitalWrite(_forwards[i], LOW);
    digitalWrite(_backwards[i], LOW);
  }
}
