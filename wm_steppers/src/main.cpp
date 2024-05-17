#include <Arduino.h>
#include <Wire.h>

//1. pin assignments
const byte kEnPin = 8;
const byte kStepXPin = 2;
const byte kDirXPin = 5;
const byte kStepYPin = 3;
const byte kDirYPin = 6; //realistically current design only needs 2 steppers
const byte kStepZPin = 4;
const byte kDirZPin = 7; //4 dc wheels mecanum

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
  digitalWrite(kEnPin, LOW); //only one en pin?
  pinMode(kStepXPin, OUTPUT); //changed to set all steppers up
  pinMode(kDirXPin, OUTPUT);
  pinMode(kStepYPin, OUTPUT);
  pinMode(kDirYPin, OUTPUT);
  pinMode(kStepZPin, OUTPUT);
  pinMode(kDirZPin, OUTPUT);
  Wire.begin(kWireAddress);
  Wire.onReceive(readData);
  Serial.begin(9600);
}

void stepperSelection(){
  byte b_step_selection[] = {kStepXPin, kStepYPin, kStepZPin};
  byte b_dir_selection[] = {kDirXPin, kDirYPin, kDirZPin};

  i_step_pin = b_step_selection[by_state];
  i_dir_pin = b_dir_selection[by_state];
}

void stepperDirection(){
  const int kJoyBuffer = kJoyNeutral/20; //2.5% buffer,
  const int kJoyBufferLower = kJoyNeutral - kJoyBuffer;
  const int kJoyBufferUpper = kJoyNeutral + kJoyBuffer;

  if ((i_stepper_direction > kJoyBufferLower) && (i_stepper_direction < kJoyBufferUpper)){
    digitalWrite(i_step_pin, LOW);
    return;
  }

  bool b_direction_forward = map(i_stepper_direction,1,255,0,1);
  digitalWrite(i_step_pin, HIGH);
  digitalWrite(i_dir_pin, b_direction_forward);
}

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
  stepperSelection(); 
  stepperDirection(); 
  stepperMoving();
}

void loop() {
  us_current_time = millis();
  stepperProcess();
}