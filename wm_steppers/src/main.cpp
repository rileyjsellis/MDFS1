#include <Arduino.h>
#include <ArmSteppers.h>
#include <Claw.h>
#include <RF24.h>
#include <SPI.h>
#include <Servo.h>
#include <TwoWheels.h>
#include <Wire.h>
#include <nRF24L01.h>

// RF24 CHANGE PINS
RF24 nrf(48, 53);  // CE, CSN previously:(9,8) //can be A0, 9
const byte linkAddress[6] =
    "link1";  // address through which two modules communicate.
const byte linkAddress2[6] = "link2";
int data[7];

// RF24 buttons
int button_count = 0;
int last_btn_count = 0;
int total_tests = 0;
int i_are_tests_up_to_date = 1;

// transmission data
unsigned long us_last_time_sent = 0;
int transmission_counter = 0;
bool b_is_transmitting = false;
bool b_data_shown = false;
int data_speed = 300;

byte state_selected;
int x_vals[15];
int y_vals[15];
int t_vals[15];
int turn_vals[15];

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
const byte kIn3 = 28;
const byte kIn4 = 26;

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

  nrf.begin();
  nrf.openReadingPipe(0, linkAddress);
  nrf.openWritingPipe(linkAddress2);  // set the address
  nrf.startListening();               // Set nrf as receiver

  Serial.begin(9600);
}

void transciever_Send_Or_Recieve() {
  if (b_is_transmitting == false) {
    if (transmission_counter >= 2) {
      b_is_transmitting = true;
      transmission_counter = 0;
      nrf.stopListening();
      us_last_time_sent = millis();
      return;
    }
    if ((millis() - us_last_time_sent) > data_speed * 4) {
      // Serial.println("error: no data recieved from remote");
      transmission_counter = 2;
    }
  }
  if (b_is_transmitting == true) {
    if (transmission_counter >= 1) {
      transmission_counter = 0;
      nrf.startListening();
      b_is_transmitting = false;
    }
    if ((millis() - us_last_time_sent) > data_speed * 4) {
      Serial.println("error: help");
      transmission_counter = 2;
    }
  }
}

void read_Data() {
  if (b_is_transmitting == false) {
    if (nrf.available()) {
      nrf.read(&data, sizeof(data));

      state_selected = data[0];
      int i = state_selected;

      x_vals[i] = data[1];
      y_vals[i] = data[2];
      t_vals[i] = data[3];
      button_count = data[4];
      i_are_tests_up_to_date = data[5];
      transmission_counter = data[6];
      b_data_shown = false;
    }
  }
}

void send_Data() {
  if (b_is_transmitting == true) {
    if ((millis() - us_last_time_sent) > data_speed / 3) {
      transmission_counter = ++transmission_counter;
      int i = state_selected;

      data[0] = state_selected;  // arm angle
      data[1] = x_vals[i];       // arm height
      data[2] = y_vals[i];
      data[3] = t_vals[i];  // diagnostic driving data
      data[4] = 1;          // holder
      data[5] = total_tests;
      data[6] = transmission_counter;

      nrf.write(data, sizeof(data));  // spit out the data array
      us_last_time_sent = millis();
      b_data_shown = false;
    }
  }
}

void Display_Terminal_Data() {
  if (b_data_shown == false) {
    if (b_is_transmitting == true) {
      Serial.print("trmt: ");
      Serial.print(data[0] + ",");
      Serial.print(data[1] + " x, ");
      Serial.print(data[2] + " y, ");
      Serial.print(data[3] + "millis, ");
      Serial.print(data[4] + "t rad, ");
      Serial.print("b, ");
      Serial.print(data[6]);
      Serial.print(". tls:  ");
      Serial.print(data[7]);
      Serial.print(". tls:  ");
      Serial.println(String(millis()));
    }
    /*
    if (b_is_transmitting == false){
      Serial.print("r: ");
      Serial.print(data[0]); Serial.print("x,");
      Serial.print(data[1]); Serial.print("y, ");
      Serial.print(int(data[2]/10.8)); Serial.print("p, ");
      Serial.print(int(data[3]/10.8)); Serial.print("°, ");
      Serial.print(data[4]); Serial.print("b, ");
      Serial.print(data[6]); Serial.print(". ");
      Serial.println(String(millis()));
    }
    */
    b_data_shown = true;
  }
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
    case 0:
      // waits for lipo to turn on
      break;
    case 1:  // to pod 1
      wheels.reverseRight(500);
      stateDuration(4000);
      break;
    case 2:  // collect pod 1
      stateDuration(4000);
      collectPod(kGround150);
      break;
    case 3:  // to pod 2
      stateDuration(4000);
      wheels.forwards();
      break;
    case 4:  // collect pod 2
      stateDuration(4000);
      collectPod(kTree150);
      break;
    case 5:  // to pod 3
      stateDuration(2000);
      wheels.forwards();
      break;
    case 6:  // collect pod 3
      stateDuration(2000);
      collectPod(kGround200);
      break;
    case 7:  // to pod 4
      wheels.reverseRight(500);
      stateDuration(8000);
    case 8:  // collect pod 4
      stateDuration(1000);
      collectPod(kGround300);
      break;
    case 9:  // to pod 5
      stateDuration(2000);
      wheels.forwards();
      break;
    case 10:  // collect pod 5
      stateDuration(2000);
      collectPod(kTree300);
      break;
    case 11:  // to pod 6
      stateDuration(3000);
      wheels.forwards();
      break;
    case 12:  // collect pod 6
      stateDuration(3000);
      collectPod(kGround200);
      break;
    case 13:  // to deposit
      stateDuration(3000);
      state = 1;  // move to actual final state
      break;
      // Serial.println(String(state) + ": deposit!!!");
  }
}

/*
const byte kEnA = 44;
const byte kIn1 = 22;
const byte kIn2 = 24;  // checked the output of these

const byte kEnB = 46;
const byte kIn3 = 28;
const byte kIn4 = 26;
*/

void leftForward() { 
  analogWrite(kEnA, 100);
  digitalWrite(kIn1, HIGH); //left forward
  digitalWrite(kIn2, LOW);

  analogWrite(kEnB,0);
  digitalWrite(kIn3, LOW);
  digitalWrite(kIn4, LOW);
}

void leftBack() {
  analogWrite(kEnA, 100);
  digitalWrite(kIn1, LOW);
  digitalWrite(kIn2, HIGH); //left back

  analogWrite(kEnB, 0);
  digitalWrite(kIn3, LOW);
  digitalWrite(kIn4, LOW);
}

void rightForward() {
  analogWrite(kEnA, 0);
  digitalWrite(kIn1, LOW);
  digitalWrite(kIn2, LOW);

  analogWrite(kEnB, 100); 
  digitalWrite(kIn3, HIGH);//right forward
  digitalWrite(kIn4, LOW);
}

void rightBack() {
  analogWrite(kEnA, 0);
  digitalWrite(kIn1, LOW);
  digitalWrite(kIn2, LOW);

  analogWrite(kEnB, 100);
  digitalWrite(kIn3, HIGH); //right back
  digitalWrite(kIn4, LOW);
}

  void loop() {

    Serial.println("left forward");
    leftForward();
    delay(2000);

    Serial.println("left back");
    leftBack();
    delay(2000);

    Serial.println("right forward");
    rightForward();
    delay(2000);

    Serial.println("right back");
    rightBack();
    delay(2000);
    // us_current_time = millis();

    // stateMachine();
    // arm.moving();

    // arm.moveTo(kGround150);

    // only while with remote testing
    //read_Data();
    //send_Data();
    //Display_Terminal_Data();
    //transciever_Send_Or_Recieve();
}