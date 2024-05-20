#include "Arduino.h"
#include "Claw.h"

Claw::Claw(byte pin) {
  pinMode(pin, OUTPUT);
  _pin = pin;

}

void Claw::open(){
  //servo to zero
}

void Claw::close(){
 //servo to 100
}