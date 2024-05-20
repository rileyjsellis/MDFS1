#include <Arduino.h>
#include <Wire.h>
#include <ArmSteppers.h>
#include <Claw.h>


//1. pin assignments
const byte kEnPin = 8;
const byte kStepXPin = 2;
const byte kDirXPin = 5;
const byte kStepYPin = 3;
const byte kDirYPin = 6;

//servo inputs for the claw. needed here
const byte kClawPin = 7; //change this

Claw claw(kClawPin);

byte allStepperPins[2][2] = {{kStepXPin, kStepYPin},
                            {kDirXPin, kDirYPin}};

//creating Arm Stepper Objects
ArmSteppers arm(allStepperPins);

//pod positions = {pod_x, pod_y}
const int kPodNut = 6;
const int kTree300[] = {200,300};
const int kTree150[] = {150,150};
const int kGround300[] = {300, kPodNut};
const int kGround200[] = {200, kPodNut}; //two of these.
const int kGround150[] = {150, kPodNut};

//2. i2c wire
const byte kWireAddress = 9;
const byte kWireArrayLength = 3;

//3. timing
unsigned long us_current_time;
unsigned long us_last_stepper_motion = 0;

//4. joy and states inputs
int i_stepper_direction = 0; //renaming for it's purpose
int i_joy_y = 0;
byte by_state = 0;
const int kJoyNeutral = 255/2; //half of byte recieved

//*** Riley's note, to review enum possibility
enum dc_motor{ 
  front_left=1,
  front_right=2,
  back_left=3,
  back_right=4
};

// stepper
int i_step_pin = kStepYPin;
int i_dir_pin = kDirYPin;

//RPM initial calculations 
const float i_steps = 400; //Nema17 specs
float f_resolution = (float)360/i_steps; //step angle 0.9deg
String s_RPM = "150"; //set s_RPM prior to code
float f_T = 0.5; //500 microseconds, assuming 150rpm, can change in code
bool b_last_step = LOW; //high low bool use case
 

void readData(int num) { 
  if (num >= kWireArrayLength) { 
    byte data[kWireArrayLength];
    for (int i = 0; i < kWireArrayLength; i++) {
      data[i] = Wire.read(); 
    }
    i_joy_y = data[0];
    i_stepper_direction = data[1];
    by_state = data[2]; 
    Serial.println(i_stepper_direction); //test
  }
}

void setup() {
  pinMode(kEnPin, OUTPUT);
  digitalWrite(kEnPin, LOW);
  Wire.begin(kWireAddress);
  Wire.onReceive(readData);
  Serial.begin(9600);
}


/*
how best to set up this code so it won't interrupt wheel movement?

send instructions to move stepper.
stepper is always 180.
stepper always wants to work to get back to 180, so changing that value.

stepper next step function
  if greater than 180, direction is negative and -- one degrees
  if less thatn 180, direction is positive and ++ one degrees

function is constantly called. new values are added all the time, SO it's a counter variable we're playing with.
*/

void stepperNextStep(){
  if (us_current_time - us_last_stepper_motion >= f_T){
    digitalWrite(i_step_pin, b_last_step);
    us_last_stepper_motion = us_current_time;
    b_last_step = !b_last_step;
  }
}

void stepperMoving(){ //current function work
  for (int i = 0; i < (i_steps * 2); i++) {
    stepperNextStep();
  }
}

float setRPM(int rpm, float res){ //assuming 150RPM, 0.9deg res
  float freq = (float)rpm/((res/360)*60); //freq = 1000
  float period = 1/freq; //period = 1/1000
  return (period*0.5)*1000; //T = 500 microseconds
}

//RPM
void readRPM(){
  if(Serial.available() > 0){
    s_RPM = Serial.readString();
    f_T = setRPM(s_RPM.toInt(), f_resolution);
    Serial.print(s_RPM);
  }
}

void stepperProcess(){
  //readRPM(); //not quite needed for now
  //stepperSelection(); 
  //stepperDirection(); //code has been removed, likely unneeded.
  stepperMoving();
}

void loop() {
  us_current_time = millis();
  //state machine checker here
  arm.moving();

}