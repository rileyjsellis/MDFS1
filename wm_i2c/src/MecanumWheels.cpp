#include "MecanumWheels.h"

#include "Arduino.h"

// creating instance of object, for all wheels (this works fine)
MecanumWheels::MecanumWheels(byte allPins[3][4]) {  // 12 pins
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 4; j++) {
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

void MecanumWheels::wheelsOn(bool answer) {
  const int speed = 255;
  for (int i = 0; i < 4; i++) {
    analogWrite(_enables[i], speed * answer);
  }
}

void MecanumWheels::goForward() {  // 0 heading
  wheelsOn(true);
  // all enables pins on
}

void MecanumWheels::goRight() {  // 90 heading
  wheelsOn(true);
  // all enables pins on
}

void MecanumWheels::goRightBackwards() {  // 135heading
  wheelsOn(true);
  // all enables pins on
}

void MecanumWheels::goLeft() {  //-90 heading
  wheelsOn(true);
  // all enables pins on
}

void MecanumWheels::spin(
    int direction) {  // typically only spin is counter clockwise, so no need to
                      // code otherwise.
  wheelsOn(true);
  // all enable pins on, direction decide.
}

void MecanumWheels::rest() {
  wheelsOn(false);
  // turn off all enables pins
}