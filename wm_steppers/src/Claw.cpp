#include "Arduino.h"
#include "Claw.h"

Claw::Claw(byte pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;

}

void Claw::open(){
  //servo 90 degrees
}

void Claw::close(){
  
}