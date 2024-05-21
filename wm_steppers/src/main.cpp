#include <Arduino.h>
#include <ArmSteppers.h>
#include <Claw.h>
#include <Wire.h>

// 1. pin assignments
const byte kEnPin = 8;
const byte kStepBasePin = 2;
const byte kDirBasePin = 5;
const byte kStepMiddlePin = 3;
const byte kDirMiddlePin = 6;

// servo inputs for the claw. needed here
const byte kClawPin = 7;  // change this

Claw claw(kClawPin);

byte allStepperPins[2][2] = {{kStepBasePin, kStepMiddlePin},
                             {kDirBasePin, kDirMiddlePin}};

// creating Arm Stepper Objects
ArmSteppers arm(allStepperPins);

// pod positions = {pod_x, pod_y}
const int kPodNut = 6;
const int kTree300[] = {200, 300};
const int kTree150[] = {150, 150};
const int kGround300[] = {300, kPodNut};
const int kGround200[] = {200, kPodNut};  // two of these.
const int kGround150[] = {150, kPodNut};

// 2. i2c wire
const byte kWireAddress = 9;
const byte kWireArrayLength = 3;

// 3. timing
unsigned long us_current_time;
unsigned long us_state_timer;
unsigned long us_time_since_pod_collection = 0;

// 4. joy and states inputs
int i_stepper_direction = 0;  // renaming for it's purpose
int i_joy_y = 0;
byte by_state = 0;
const int kJoyNeutral = 255 / 2;  // half of byte recieved

// 5. state machine specific code
byte last_state = 0;
byte state = 0;
byte pod_collection_state = 0;

//*** Riley's note, to review enum possibility
enum dc_motor {
  front_left = 1,
  front_right = 2,
  back_left = 3,
  back_right = 4
};

// stepper
int i_step_pin = kStepMiddlePin;
int i_dir_pin = kDirMiddlePin;

// RPM initial calculations
const float i_steps = 200;  // Nema specs //should this all be in Arm Stepper??
float f_resolution = (float)360 / i_steps;  // step angle 0.9deg
String s_RPM = "150";                       // set s_RPM prior to code
float f_T = 0.5;  // 500 microseconds, assuming 150rpm, can change in code
bool b_last_step = LOW;  // high low bool use case

void readData(int num) {
  if (num >= kWireArrayLength) {
    byte data[kWireArrayLength];
    for (int i = 0; i < kWireArrayLength; i++) {
      data[i] = Wire.read();
    }
    i_joy_y = data[0];
    i_stepper_direction = data[1];
    by_state = data[2];
    Serial.println(i_stepper_direction);  // test
  }
}

void setup() {
  pinMode(kEnPin, OUTPUT);
  digitalWrite(kEnPin, LOW);
  Wire.begin(kWireAddress);
  Wire.onReceive(readData);
  Serial.begin(9600);
}

void stepperNextStep() {
  if (us_current_time - us_last_stepper_motion >= f_T) {
    digitalWrite(i_step_pin, b_last_step);
    us_last_stepper_motion = us_current_time;
    b_last_step = !b_last_step;
  }
}

void stepperMoving() {  // current function work
  for (int i = 0; i < (i_steps * 2); i++) {
    stepperNextStep();
  }
}

float setRPM(int rpm, float res) {               // assuming 150RPM, 0.9deg res
  float freq = (float)rpm / ((res / 360) * 60);  // freq = 1000
  float period = 1 / freq;                       // period = 1/1000
  return (period * 0.5) * 1000;                  // T = 500 microseconds
}

// RPM
void readRPM() {
  if (Serial.available() > 0) {
    s_RPM = Serial.readString();
    f_T = setRPM(s_RPM.toInt(), f_resolution);
    Serial.print(s_RPM);
  }
}

void stepperProcess() {
  // readRPM(); //not quite needed for now
  // stepperSelection();
  // stepperDirection(); //code has been removed, likely unneeded.
  stepperMoving();
}

// ensures timer is only reset once for each state.
void stateDuration(int timing) {
  if (state != last_state) {
    us_state_timer = us_current_time + timing;
  }
}

// This progresses all states.
void stateProgression() {
  if (us_state_timer < us_current_time && state != 0) {
    if (pod_collection_state != 0) {
      pod_collection_state++;
    } else {
      state++;
    }
  }
}

// Reduces repetitive states required in main state machine.
void collectPod(const int collect_pod[2]) {
  switch (pod_collection_state) {
    case 0:  // if it's been over a second since last pod collection
      if (us_current_time > (us_time_since_pod_collection + 2000)) {
        pod_collection_state++;
      }
    case 1:
      stateDuration(500);
      claw.open();
    case 2:
      stateDuration(1000);
      arm.moveTo(collect_pod);
    case 3:
      stateDuration(500);
      claw.close();
    case 4:
      stateDuration(1000);
      arm.deposit();
    case 5:
      us_time_since_pod_collection = us_current_time;
      pod_collection_state = 0;
  }
}

void stateMachine() {
  switch (state) {
    case 0:  // await button press to begin track run
             // if button pressed = true: state++;
    case 1:
      // move to platform
      stateDuration(1000);
    case 2:
      // move to first pod
      stateDuration(500);
    case 3:
      stateDuration(200);
      collectPod(kGround150);
    case 4:
      // move to 1st tree
    case 5:
      collectPod(kTree150);
    case 6:
      // move to ground pod
    case 7:
      collectPod(kGround200);
    case 8:
      // travel across to next platform
    case 9:
      collectPod(kGround300);
    case 10:
      // move to 2nd tree
    case 11:
      collectPod(kTree300);
    case 12:
      // move to final pod
    case 13:
      collectPod(kGround200);
    case 14:
      // move to deposit hole
  }
  stateProgression();
}

void loop() {
  us_current_time = millis();

  stateMachine();
  arm.moving();
}