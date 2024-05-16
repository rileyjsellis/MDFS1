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
unsigned long us_state_duration;

//4. joy and states inputs
int i_joy_x = 0;
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

// all other global variables
int i_step_pin = kStepYPin;
int i_dir_pin = kDirYPin;

//RPM initial calculations 
const float i_steps = 400; //Nema17 specs
float f_resolution = (float)360/steps; //step angle 0.9deg
String s_RPM = 150; //set s_RPM prior to code
float f_T = 0.5; //500 microseconds, assuming 150rpm, can change in code
bool b_last_step = LOW; //high low bool use case
 
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

void loop() {
    us_current_time = millis();
    stepperProcess();
}

void readData(byte num) { 
  if (num >= kWireArrayLength) { 
    byte data[kWireArrayLength];
    for (int i = 0; i < kWireArrayLength; i++) {
      data[i] = Wire.read(); 
    }
    i_joy_y = data[0];
    i_joy_x = data[1];
    by_state = data[2]; 
    Serial.println(i_joy_x); //test
  }
}

void stepperProcess(){
    //readRPM(); //not quite needed for now
    stepperSelection(); 
    stepperDirection(); 
    stepperMoving();
}

/*
Riley's code plan to remove delays from code

we need to reasses what jayden's machine is attempting. 
we need to recreate it with:
1. no copy paste, more modular code
2. millis() rather than delays
3. add s_RPM

*/

void stepperSelection(){
    switch (by_state){
        case 0: //Riley's note, I've changed around the case order, x,y,z made more sense than y,x,z.
            i_step_pin=kStepXPin;
            i_dir_pin=kDirXPin;
            break;
        case 1:
            i_step_pin=kStepYPin;
            i_dir_pin=kDirYPin;
            break;
        case 2:
            i_step_pin=kStepZPin;
            i_dir_pin=kDirZPin;
            break;
        default:
            Serial.println("error: state num out of range");
            break;
    }
}

void stepperDirection(){
    const int kJoyBuffer = kJoyNeutral/20; //2.5% buffer,
    const int kJoyBufferLower = kJoyNeutral - kJoyBuffer;
    const int kJoyBufferUpper = kJoyNeutral + kJoyBuffer;

    if ((i_joy_x > kJoyBufferLower) && (i_joy_x < kJoyBufferUpper)){
        digitalWrite(i_step_pin, LOW);
        return;
    }
    int step_direction = i_joy_x - kJoyNeutral;
    if (step_direction > 0){
        digitalWrite(i_dir_pin, LOW);
    }
    else {
        digitalWrite(i_dir_pin, HIGH);
    }
}

void stepperNextStep(){
    const int i_millis_btwn_steps = 0.5; //500 microseconds

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

//RPM
void readRPM(){
    if(Serial.avaliable() > 0){
        s_RPM = Serial.readString();
        f_T = setRPM(s_RPM.toInt(), f_resolution);
        Serial.print(s_RPM);
    }
}

void setRPM(int rpm, float res){ //assuming 150RPM, 0.9deg res
    float freq = (float)rpm/((res/360)*60); //freq = 1000
    float period = 1/freq; //period = 1/1000
    return (period*0.5)*1000; //T = 500 microseconds
}

//Jayden's Prior Function, all in one
void jaydensStateMachine(){
    float i_pulse_width_mircos = 100;
    float i_millis_btwn_steps = 1000;
    if (by_state == 0) { //RN: stepperSelection()
       i_step_pin=kStepYPin;
       i_dir_pin=kDirYPin; 

    if (i_joy_x < 490) { //high low changes direction.
      digitalWrite(i_dir_pin, LOW); //Changes the rotations direction
      //i_steps = map(i_joy_x, 0, 499, 1, 10); 
      for (int i = 0; i < i_steps; i++) {
        digitalWrite(i_step_pin, HIGH);
        delayMicroseconds(i_pulse_width_mircos);
        digitalWrite(i_step_pin, LOW);
        delayMicroseconds(i_millis_btwn_steps); 
      }
    } else if (i_joy_x > 510) {
      digitalWrite(i_dir_pin, HIGH); //Changes the rotations direction
      //i_steps = map(i_joy_x, 503, 1023, 1, 10); 
      for (int i = 0; i < i_steps; i++) {
        digitalWrite(i_step_pin, HIGH);
        delayMicroseconds(i_pulse_width_mircos);
        digitalWrite(i_step_pin, LOW);
        delayMicroseconds(i_millis_btwn_steps);
      }
    } else {
      digitalWrite(i_step_pin, LOW);
    }
  } else if (by_state == 1) {
    i_step_pin=kStepXPin;
    i_dir_pin=kDirXPin;
     if (i_joy_x < 495) {
      digitalWrite(i_dir_pin, LOW); //Changes the rotations direction //NOTE it's a method to swap direction.
      //i_steps = map(i_joy_x, 0, 499, 1, 10); 
      for (int i = 0; i < i_steps; i++) {
        digitalWrite(i_step_pin, HIGH);
        delayMicroseconds(i_pulse_width_mircos);
        digitalWrite(i_step_pin, LOW);
        delayMicroseconds(i_millis_btwn_steps); 
      }
    } else if (i_joy_x > 505) {
      digitalWrite(i_dir_pin, HIGH); //Changes the rotations direction
      //i_steps = map(i_joy_x, 503, 1023, 1, 10); 
      for (int i = 0; i < i_steps; i++) {
        digitalWrite(i_step_pin, HIGH);
        delayMicroseconds(i_pulse_width_mircos);
        digitalWrite(i_step_pin, LOW);
        delayMicroseconds(i_millis_btwn_steps);
      }
    } else {
      digitalWrite(i_step_pin, LOW);
    }
    i_step_pin=kStepZPin;
    i_dir_pin=kDirZPin;
     if (i_joy_x < 495) {
      digitalWrite(i_dir_pin, LOW); //Changes the rotations direction
      //i_steps = map(i_joy_x, 0, 499, 1, 10); 
      for (int i = 0; i < i_steps; i++) {
        digitalWrite(i_step_pin, HIGH);
        delayMicroseconds(i_pulse_width_mircos);
        digitalWrite(i_step_pin, LOW);
        delayMicroseconds(i_millis_btwn_steps); 
      }
    } else if (i_joy_x > 505) {
      digitalWrite(i_dir_pin, HIGH); //Changes the rotations direction
      //i_steps = map(i_joy_x, 503, 1023, 1, 10); 
      for (int i = 0; i < i_steps; i++) {
        digitalWrite(i_step_pin, HIGH);
        delayMicroseconds(i_pulse_width_mircos);
        digitalWrite(i_step_pin, LOW);
        delayMicroseconds(i_millis_btwn_steps);
      }
    } else {
      digitalWrite(i_step_pin, LOW);
    }
  }
}