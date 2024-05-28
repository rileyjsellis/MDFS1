// Rileys code using stepper library rather than individual effort.

#include <AccelStepper.h>
#include <Arduino.h>
#include <Servo.h>

const int enPin = 8;
const int stepXPin = 2;  // X.STEP //X is winch
const int dirXPin = 5;   // X.DIR
const int stepYPin = 3;  // Y.STEP //Y is left bottom
const int dirYPin = 6;   // Y.DIR
const int stepZPin = 4;  // Z.STEP
const int dirZPin = 7;   // Z.DIR //z right bottom

const int servoConnected = 40;
// dc motors

const byte kIn1 = 22;
const byte kIn2 = 24;  // checked the output of these

const byte kIn3 = 26;
const byte kIn4 = 28;

const int SERVO_PIN = 32;  // adjusted to the current outcome

const int start_btn = 8;  // implmented button //was 30

bool b_last_btn = 0;
bool b_btn_state = 0;

int state = 0;
int last_state = 0;
int servoOff = 0;

bool wheels_active;

AccelStepper winch(AccelStepper::DRIVER, stepXPin, dirXPin);
AccelStepper leftBase(AccelStepper::DRIVER, stepYPin, dirYPin);
AccelStepper rightBase(AccelStepper::DRIVER, stepZPin, dirZPin);

Servo servo;
bool system_ready = 1;  // state 0 open, state 1 closed

// timing temporary
unsigned long current_time;
unsigned long last_moved = 0;
unsigned long drive_timer = 0;
unsigned long last_wheel_adjustment = 0;
bool b_wheel_on = true;
int driving = 0;
int last_driving = 0;

// btn timing
unsigned long us_current_time;
unsigned long us_time_since_last_send = 0;
unsigned long us_last_debounce_time = 0;

void setup() {
  winch.setMaxSpeed(100);
  winch.setAcceleration(300);
  leftBase.setMaxSpeed(100);
  leftBase.setAcceleration(200);
  rightBase.setMaxSpeed(100);
  rightBase.setAcceleration(200);
  servo.attach(SERVO_PIN);
  pinMode(start_btn, INPUT);
  digitalWrite(start_btn, HIGH);
  pinMode(kIn1, OUTPUT);
  pinMode(kIn2, OUTPUT);
  pinMode(kIn3, OUTPUT);
  pinMode(kIn4, OUTPUT);
  pinMode(servoConnected,HIGH);

  // checked the output of these
}

void stateProgressionCheck() {
  if (winch.distanceToGo() == 0 && leftBase.distanceToGo() == 0 &&
      rightBase.distanceToGo() == 0 && wheels_active == false) {
    state++;
  }
}

void servoClose() {
  // if (system_ready == 1) {
  servo.write(110);  // these are to change.
  //}
}

void servoOpen() {
  // if (system_ready == 0) {
  servo.write(45);
  //}
}

void servoOpenClose() {
  if (current_time > last_moved + 5000) {
    last_moved = current_time;
    system_ready = !system_ready;
  }
}

void readButton() {
  const byte kDebounceDelay = 50;
  int reading = digitalRead(start_btn);

  if (reading != b_last_btn) {
    us_last_debounce_time = us_current_time;
  }

  if ((us_current_time - us_last_debounce_time) > kDebounceDelay) {
    if (reading != b_btn_state) {
      b_btn_state = reading;
      if (b_btn_state == LOW) {
        state++;
        Serial.println(b_last_btn);
      }
    }
  }
  b_last_btn = reading;
}

void moveArm(int wi, int base) {
  winch.moveTo((long)wi);
  leftBase.moveTo((long)base);
  rightBase.moveTo((long)(-1 * base));
}

void forward() {
  digitalWrite(kIn1, HIGH);
  digitalWrite(kIn2, LOW);
  digitalWrite(kIn3, LOW);
  digitalWrite(kIn4, HIGH);
  /*
  if (current_time > last_wheel_adjustment + 1000) {
    b_wheel_on = !b_wheel_on;
    if (b_wheel_on == true) {
      digitalWrite(kIn3, HIGH);
      digitalWrite(kIn4, LOW);
    } else {
      digitalWrite(kIn3, LOW);
      digitalWrite(kIn4, LOW);
    }
  }
  */
}

void wheels_off() {
  digitalWrite(kIn1, LOW);
  digitalWrite(kIn2, LOW);
  digitalWrite(kIn1, LOW);
  digitalWrite(kIn2, LOW);
}

void loop() {
  delay(5000);
  current_time = millis();
  // Serial.println(String(digitalRead(start_btn)

  if (current_time > last_moved + 500) {
    system_ready = !system_ready;
  }
  //forward();
  
  if (digitalRead(servoConnected) == HIGH) {
    if (millis() >= servoOff + 100) {
      double servoOff = millis();
      digitalWrite(servoConnected, LOW);
    }
  }

  if (state == 0) {  // steps for 150 pod collection
  } else if (state == 1) {
    moveArm(-750, 0);
  } else if (state == 2) {
    system_ready = 0;
    moveArm(-770, 320);
  } else if (state == 3) {
    moveArm(100, -450);  // deposit positon!!!!
  } else if (state == 4) {
    wheels_active = true;
    drive_timer = 500;
    digitalWrite(kIn1, HIGH);
    digitalWrite(kIn2, LOW);
    digitalWrite(kIn3, HIGH);
    digitalWrite(kIn4, LOW);

  } else if (state == 4) {
    // moveArm(200, -50);
  }
  if (state != last_state) {
    Serial.println("state change");
  }
  last_state = state;

  while (winch.distanceToGo() != 0 || leftBase.distanceToGo() != 0 ||
         rightBase.distanceToGo() != 0) {
    winch.run();
    leftBase.run();
    rightBase.run();
  }

  stateProgressionCheck();

  // readButton();
  // servoClose();
  // servoOpen();
}