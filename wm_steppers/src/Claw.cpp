#include "Arduino.h"
#include "Claw.h"

Claw::Claw(byte pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;

}


void Claw::open(){
  
}

void Claw::close(){
  
}