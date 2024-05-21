#ifndef ArmSteppers_h
#define ArmSteppers_h
#include "Arduino.h"

// Code Overview:
// This code delcares the class for Arm Stepper Motors, to use, we simply
// enter the x and y values of a pod's location, these are in main.cpp
// for example: the pod in the 300mm tree is at position (200,300).

// Calculations:
// We make use of simple trigonmetric calculations, as all sides are known,
// to get the angle's required for each stepper to reach it's desired location.

class ArmSteppers {
 public:
  ArmSteppers(byte allStepperPins[2][2]);
  void moveTo(const int pod_pos[2]);
  void deposit();
  void moving();

 private:
  float getSideCLength();
  void adjustXYvals(const int pod_pos[2]);
  float getBaseAxisHeading();
  float getMiddleAxisHeading();

  byte _pinMotion[2];
  byte _pinDirection[2];

  int _pod_pos[2];  // x and y inputs

  // Trig Values used in Multiple private functions
  const float _kArmSideA = 300;     // side a, lower arm
  const float _kArmSideB = 250.24;  // side b, upper arm
  float _dist_side_c;               // side c, distance to pod

  // Private alterations to x and y inputs, designed to be altered as chassis
  // and arm aren't finalised yet.
  const int _kNodeXToPlatformX = 130;
  const int _kClawOverPodY = 90;
  const int _kBaseNodeY = 50;
  const int _kAddY = _kClawOverPodY - _kBaseNodeY;  // = 40 y total

  // To simplify readability in ArmStepper functions.
  const bool kBaseAxis = 0;
  const bool kMiddleAxis = 1;
  bool b_base_or_middle = 0;

  enum stepper {
    base = 0,
    middle = 1,
  };

  enum direction {
    base = 0,
    middle = 1,
  };

  // Main Values for moving the arms to next state.
  int _armheading[2] = {0, 0};
  int _armcurrent[2] = {0, 0};

  // timing
  unsigned long us_last_stepper_motion = 0;
};

#endif