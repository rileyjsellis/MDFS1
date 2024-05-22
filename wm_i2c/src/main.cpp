// code for joystick and for the wheels L298N systems

#include <Arduino.h>
#include <TwoWheels.h>
#include <Wire.h>

// 1. joystick pin assignment
const byte kJoyXPin = A0;
const byte kJoyYPin = A1;
const byte kJoyBtnPin = 12;

// 2. DC pin assignment
const byte kEnA = 9;
const byte kIn1 = 2;
const byte kIn2 = 3;

const byte kEnB = 10;
const byte kIn3 = 4;
const byte kIn4 = 5;

byte allPins[3][2] = {{kEnA, kEnB}, {kIn1, kIn3}, {kIn2, kIn4}};

// creating an object for all dc wheels

TwoWheels wheels(allPins);

// 3. i2c wire
const byte kWireAddress = 9;
const byte kWireArrayLength = 3;

// 4. timing
unsigned long us_current_time;
unsigned long us_time_since_last_send = 0;
unsigned long us_last_debounce_time = 0;

// 5. joystick and btn inputs
byte i_joy_x = 0;
byte i_joy_y = 0;
byte by_state = 0;
bool b_last_btn = 0;
bool b_btn_state = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(kJoyXPin, INPUT);
  pinMode(kJoyYPin, INPUT);
  pinMode(kJoyBtnPin, INPUT_PULLUP);
  digitalWrite(kJoyBtnPin, HIGH);
}

void readInputs() {
  i_joy_x = byte(analogRead(map(kJoyXPin, 1, 1024, 1, 255)));
  i_joy_y = byte(analogRead(map(kJoyYPin, 1, 1024, 1, 255)));
  Serial.print(digitalRead(kJoyBtnPin));
}

void keepStateWitihinRange() {
  const byte num_steppers = 2;
  if (by_state == num_steppers) {
    by_state = 0;
  }
}

void readButton() {
  const byte kDebounceDelay = 50;
  int reading = digitalRead(kJoyBtnPin);

  if (reading != b_last_btn) {
    us_last_debounce_time = us_current_time;
  }

  if ((us_current_time - us_last_debounce_time) > kDebounceDelay) {
    if (reading != b_btn_state) {
      b_btn_state = reading;
      if (b_btn_state == LOW) {
        by_state++;
        keepStateWitihinRange();
      }
    }
  }
  b_last_btn = reading;
}

void sendData() {
  byte data[kWireArrayLength];
  data[0] = i_joy_x;
  data[1] = i_joy_y;
  data[2] = by_state;
  Serial.println(data[2]);
  Wire.beginTransmission(kWireAddress);
  Wire.write(data, kWireArrayLength);
  Wire.endTransmission();
}

void collectAndSendData() {
  const int kSendingInterval = 300;
  if (us_current_time - us_time_since_last_send > kSendingInterval) {
    readInputs();
    sendData();
    us_time_since_last_send = us_current_time;
  }
}

// state system will exist here and SEND over to the stepper arduino. This is
// now the guide

// state machine requires 1) actions that play out, 2) duration for each step.
// //only wheels state machine will occur here, stepper on other arduino.

void loop() {
  us_current_time = millis();
  readButton();
  collectAndSendData();
}