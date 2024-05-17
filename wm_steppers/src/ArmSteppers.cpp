#include "Arduino.h"
#include "ArmSteppers.h"

ArmSteppers::ArmSteppers(byte allStepperPins[2][2]) {
  for (int i = 0; i < 2; i++){
    for (int j = 0; j < 2; j++){
      pinMode(allStepperPins[i][j], OUTPUT);
      switch (i){
        case 0:
          _pinMotion[j] = allStepperPins[i][j];
        case 1:
          _pinDirection[j] = allStepperPins[i][j];
      }
    }
  }
}

void ArmSteppers::moveTo(int topPosition, int lowerPosition){
  int intendedHeadings[2] = {topPosition, lowerPosition};
  for (int i = 0; i < 2; i++){
    _armheading[i] = _armheading[i] + intendedHeadings[2];
  }
}

void ArmSteppers::moving(){
//constant checking
 if (_armheading > _armcurrent){
    //void function direction move arm --
  }
  if (_armheading < _armcurrent){
    //void function direction move arm ++
  }
  //here RPM code will go.
}