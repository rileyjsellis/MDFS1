#include "Arduino.h"
#include "ArmSteppers.h"

ArmSteppers::ArmSteppers(byte allStepperPins[2][2])
{
  for (int i = 0; i < 2; i++)
  {
    for (int j = 0; j < 2; j++)
    {
      pinMode(allStepperPins[i][j], OUTPUT);
      switch (i)
      {
      case 0:
        _pinMotion[j] = allStepperPins[i][j];
      case 1:
        _pinDirection[j] = allStepperPins[i][j];
      }
    }
  }
}

// This function adjust's the user's desired inputs.

void ArmSteppers::adjustXYvals(int pod_pos[2])
{
  for (int i = 0; i < 2; i++)
  {
    _pod_pos[i] = pod_pos[i];
  }
  _pod_pos[0] = _pod_pos[0] + _kNodeXToPlatformX; // adding x val to pod distance.
  _pod_pos[1] = _pod_pos[0] + _kAddY;             // adding y to consider claw height and initial stepper height.
}

// Trig calculations to get the base angle
float ArmSteppers::getBaseAxisHeading()
{
  _side_c_to_pos = sqrt(_pod_pos[0] * _pod_pos[0] + _pod_pos[1] * _pod_pos[1]); // hypotenuse

  // x and y initial
  float _theta_b_1 = atan2(_pod_pos[1], _pod_pos[2]) * 180 / (3.14);
  float _theta_b_2 = acos((_kArmA * _kArmA + _side_c_to_pos * _side_c_to_pos - _kArmB * _kArmB) / (2 * _kArmA * _side_c_to_pos)); // angle within created triangle

  float baseAxisHeading = (180 - (_theta_b_1 + _theta_b_2));

  return baseAxisHeading;
}

// Trig calculations to get stepper that alters the middle arm node.
float ArmSteppers::getMiddleAxisHeading()
{

  float _theta_c = acos((_kArmA * _kArmA + _kArmB * _kArmB - _side_c_to_pos * _side_c_to_pos) / (2 * _kArmA * _kArmB));

  float middleAxisHeading = (180 - (_theta_c));

  return middleAxisHeading; // 180 minus angles
}

// This is the code the user will input for each pod and deposit, giving only x and y.
void ArmSteppers::moveTo(int pod_pos[2])
{
  adjustXYvals(pod_pos);
  _armheading[kBaseAxis] = getBaseAxisHeading();
  _armheading[kMiddleAxis] = getMiddleAxisHeading();
}

// This will occur each loop without user input.
void ArmSteppers::moving()
{

  if (_armheading[b_base_or_middle] > _armcurrent[b_base_or_middle])
  {
    // void function direction move arm --
  }
  if (_armheading[b_base_or_middle] < _armcurrent[b_base_or_middle])
  {
    // void function direction move arm ++
  }
  b_base_or_middle = !b_base_or_middle; // alternates which stepper is on, giving the other a delay time.
  // here RPM code will go.
}