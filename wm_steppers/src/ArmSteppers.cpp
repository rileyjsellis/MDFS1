#include "ArmSteppers.h"

#include "Arduino.h"

ArmSteppers::ArmSteppers(byte allStepperPins[2][2]) {
  for (int i = 0; i < 2; i++) {
    for (int j = 0; j < 2; j++) {
      pinMode(allStepperPins[i][j], OUTPUT);
      switch (i) {
        case 0:
          _pinMotion[j] = allStepperPins[i][j];
        case 1:
          _pinDirection[j] = allStepperPins[i][j];
      }
    }
  }
}

// Private: Changes x and y inputs include robot's base axis distancesg that
// will change the distance the arm has to extend.
void ArmSteppers::adjustXYvals(const int pod_pos[2]) {
  for (int i = 0; i < 2; i++) {
    _pod_pos[i] = pod_pos[i];
  }
  // changes due to distance from platform.
  _pod_pos[0] = _pod_pos[0] + _kNodeXToPlatformX;
  // changes due to claw height and initial stepper height.
  _pod_pos[1] = _pod_pos[0] + _kAddY;
}

// Private: Trig calculations to output the intended base axis angle.
float ArmSteppers::getBaseAxisHeading() {
  // x and y initial
  float _theta_b_1 = atan2(_pod_pos[1], _pod_pos[2]) * 180 / (3.14);
  // angle within created triangle
  float _theta_b_2 =
      acos((_kArmSideA * _kArmSideA + _dist_side_c * _dist_side_c -
            _kArmSideB * _kArmSideB) /
           (2 * _kArmSideA * _dist_side_c));
  float baseAxisHeading = (180 - (_theta_b_1 + _theta_b_2));

  return baseAxisHeading;
}

// Private: Trig calculations to output the intended middle axis angle.
float ArmSteppers::getMiddleAxisHeading() {
  float _theta_c = acos((_kArmSideA * _kArmSideA + _kArmSideB * _kArmSideB -
                         _dist_side_c * _dist_side_c) /
                        (2 * _kArmSideA * _kArmSideB));

  float middleAxisHeading = (180 - (_theta_c));

  return middleAxisHeading;
}

// Private: Finds distance arm must travel from the given x and y inputs
float ArmSteppers::getSideCLength() {
  float sideCLength = sqrt(_pod_pos[kBaseAxis] * _pod_pos[kBaseAxis] +
                           _pod_pos[kMiddleAxis] * _pod_pos[kMiddleAxis]);
  return sideCLength;
}

// Public: User will call this function for each pod collection.
void ArmSteppers::moveTo(const int pod_pos[2]) {
  adjustXYvals(pod_pos);
  _dist_side_c = getSideCLength();
  _armheading[kBaseAxis] = getBaseAxisHeading();
  _armheading[kMiddleAxis] = getMiddleAxisHeading();
}

// Public: moveTo function specifically for each deposit, may need to have
// adjustXYvals removed, testing will reveal outcomes.
void ArmSteppers::deposit() {
  const int _deposit_x_y[2] = {0, 50};  // initial test values, to change.
  moveTo(_deposit_x_y);
}

// Public: User places this in their main loop, working off millis() it
// will adjust both arms from current position to the desired heading position.
void ArmSteppers::moving() {
  if (_armheading[b_base_or_middle] > _armcurrent[b_base_or_middle]) {
    // void function direction move arm --
  }
  if (_armheading[b_base_or_middle] < _armcurrent[b_base_or_middle]) {
    // void function direction move arm ++
  }
  b_base_or_middle = !b_base_or_middle;  // alternates which stepper is on,
                                         // giving the other a delay time.
  // here RPM code will go.
}