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
    void moveTo(int pod_pos[2]);
    void moving();

  private:
    void adjustXYvals(int pod_pos[2]);
    float getBaseAxisHeading();
    float getMiddleAxisHeading();

    byte _pinMotion[2];
    byte _pinDirection[2];

    int _pod_pos[2]; //x and y inputs

    //Trig Calculations
    const float _kArmA = 300; //side a, lower arm
    const float _kArmB = 250.24; //side b, upper arm

    float _side_c_to_pos; // side c
    float _angleA;
    float _angleB; //for first axis, between a and c
    float _angleC; //for second axis, between a and b

    // Alterations to x and y position inputs
    // This is designed to be altered as the final robot chassis and arm are finalised yet.
    const int _kNodeXToPlatformX = 130;
    const int _kClawOverPodY = 90; //these two cancel out to be 40, create new const.
    const int _kBaseNodeY = 50;
    const int _kAddY = _kClawOverPodY - _kBaseNodeY;

    //Values to simplify code readability in ArmSteppers.cpp
    const bool kBaseAxis = 0;
    const bool kMiddleAxis = 1;
    bool b_base_or_middle = 0;

    //Main Values to determine arm steps remaining.
    byte _armheading[2] = {0,0};
    byte _armcurrent[2] = {0,0}; //starts at 0 degrees, check solidworks calculations.
};

#endif