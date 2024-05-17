#include "Arduino.h"
#include "ArmStepper.h"

ArmStepper::ArmStepper(byte pinMotion, byte pinDirection) {
  pinMode(pinMotion, OUTPUT);
  pinMode(pinDirection, OUTPUT);
  _pinMotion = pinMotion;
  _pinDirection = pinDirection;
}

void ArmStepper::moveTo(int angle){
  _armheading = _armheading + angle;
  if (_armheading > _armcurrent){
    //void function direction move arm --
  }
  if (_armheading < _armcurrent){
    //void function direction move arm ++
  }
  
}

void ArmStepper::moving(){ //constant passive timing arm movement to adjust
  //add all relevant constants and timing here.
  //here RPM code will go.
}