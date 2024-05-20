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

void ArmSteppers::moveTo(int pod_pos[2]){
  for(int i = 0; i < 2; i++){
    _pod_pos[i] = pod_pos[i];
  }
  _pod_pos[0] = _pod_pos[0] + _kNodeXToPlatformX; //x val
  _pod_pos[1] = _pod_pos[0] + _kAddY; //y val
  _hyp_to_pos = sqrt(_pod_pos[0]*_pod_pos[0]+_pod_pos[1]*_pod_pos[1]); //hypotenuse
  _theta = atan2(_pod_pos[1],_pod_pos[2]) * 180/3.14; //get angle

  //going to use the hypotenuse to work out the angle for both arms.
  //going to use the _theta to Add to the position heading for both angles
  
  int intendedHeadings[2]; //both stepper angles

  for (int i = 0; i < 2; i++){
    _armheading[i] = _armheading[i] + intendedHeadings[i];
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