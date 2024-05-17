#include "Arduino.h"
#include "ArmStepper.h"

ArmStepper::ArmStepper(byte pinMotion, byte pinDirection) {
  pinMode(pinMotion, OUTPUT);
  pinMode(pinDirection, OUTPUT);
  _pinMotion = pinMotion;
  _pinDirection = pinDirection;
}

void ArmStepper::moveTo(int angle){
  
}

void ArmStepper::moving(){ //constant passive timing arm movement to adjust
  
}