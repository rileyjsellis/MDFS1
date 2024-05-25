#include <Arduino.h>
#include <ArmSteppers.h>
#include <Claw.h>
#include <TwoWheels.h>
#include <Wire.h>

// 1. pin assignments
const byte kEnPin = 8;
const byte kStepBasePin = 2;
const byte kDirBasePin = 5;
const byte kStepMiddlePin = 3;
const byte kDirMiddlePin = 6;

// DC MOTOR MOVING INTO THIS CODE
//  2. DC pin assignment
const byte kEnA = 44;
const byte kIn1 = 22;
const byte kIn2 = 24;  // checked the output of these

const byte kEnB = 46;
const byte kIn3 = 26;
const byte kIn4 = 28;

// byte allPins[3][2] = {{kEnA, kEnB}, {kIn1, kIn3}, {kIn2, kIn4}}; //with all
byte allPins[3][2] = {{kEnA, kEnB}, {kIn1, kIn3}, {kIn2, kIn4}};

// creating an object for all dc wheels

TwoWheels wheels(allPins);

// servo inputs for the claw. needed here
const byte kClawPin = 7;  // change this

Claw claw(kClawPin);

byte allStepperPins[2][2] = {{kStepBasePin, kStepMiddlePin},
                             {kDirBasePin, kDirMiddlePin}};

// creating arm stepper object
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
byte state = 1;  // change once we have a start switch
byte last_state =
    30;  // just to offset the initial states, maybe make it out of range
byte pod_state = 0;
byte last_pod_state = last_state;

// stepper
int i_step_pin = kStepMiddlePin;
int i_dir_pin = kDirMiddlePin;

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

// Ensures time is only seen once per state.
void stateDuration(int timing) {
  if (state != last_state || pod_state != last_pod_state) {
    us_state_timer = us_current_time + timing;
    // Serial.println("reset reached!! state:" + String(state) + " time:" +
    // String(millis())); //test for state change
  }
  last_state = state;
  last_pod_state = pod_state;
}

// This progresses all states after given timer.
void stateProgression() {
  if (us_state_timer <= us_current_time && state != 0) {
    Serial.println(String(state));
    if (pod_state != 0) {
      pod_state++;
    } else {
      state++;
    }
  }
}

// Repeated tasks for collecting and depositing a single pod.
void collectPod(const int collect_pod[2]) {
  switch (pod_state) {
    case 0:
      // timer prevents a loop
      if (us_current_time >= (us_time_since_pod_collection + 2000)) {
        pod_state++;
      }
      break;
    case 1:
      // Serial.println("cstate 1: open");
      stateDuration(500);
      claw.open();
      break;
    case 2:
      stateDuration(1000);
      // Serial.println("cstate 2: move specific pod");
      arm.moveTo(collect_pod);  // move towards specific pod
      break;
    case 3:
      stateDuration(500);
      // Serial.println("cstate 3: close claw");
      break;
      claw.close();
    case 4:  // move to deposit location
      stateDuration(1000);
      // Serial.println("cstate 4: deposit ");
      arm.deposit();
    case 5:
      // Serial.println("cstate 5: open");
      claw.open();  // release pod into chassis path
      us_time_since_pod_collection = us_current_time;
      pod_state = 0;
      break;
  }
}

// Main State Machine. Testing required but relies on timing to reach each
// location.
void stateMachine() {
  stateProgression();
  switch (state) {
    case 0:  // waits for button press to begin track.
             // add a switch to this later on
      break;
    case 1:
      wheels.turnRight(5);
      stateDuration(2000);
      break;
    case 2:
      wheels.forwards();
      stateDuration(500);
      break;
    case 3:
      stateDuration(200);
      collectPod(kGround150);
      break;
    case 4:
      // move to first tree
      stateDuration(1000);
      wheels.forwards();
      break;
    case 5:
      stateDuration(500);
      collectPod(kTree150);
      break;
    case 6:
      // move to ground pod
      stateDuration(1000);
      wheels.forwards();
      break;
    case 7:
      stateDuration(1000);
      collectPod(kGround200);
      break;

    // here is the biggest problem area, multiple steps required to move the
    // robot fully from one side to the other.
    case 8:
      stateDuration(1000);

      wheels.backwards();
      // travel across to next platform
      break;

    case 9:
      stateDuration(2000);
      wheels.turnLeft(5);
      break;

    case 10:
      wheels.backwards();
      stateDuration(500);
      break;

    case 11:
      stateDuration(1000);
      collectPod(kGround300);
      break;
    case 12:
      stateDuration(1000);
      wheels.forwards();
      break;
      // move to 2nd tree
    case 13:
      stateDuration(1000);
      collectPod(kTree300);
      break;
    case 14:
      // move to final pod
      stateDuration(1000);
      wheels.forwards();
      break;
    case 15:
      stateDuration(1000);
      collectPod(kGround200);
      break;
    case 16:
      // move to deposit hole
      stateDuration(1000);
      state = 1;  // move to actual final state
      break;
      // Serial.println(String(state) + ": deposit!!!");
  }
}

void loop() {
  us_current_time = millis();

  stateMachine();
  arm.moving();
}