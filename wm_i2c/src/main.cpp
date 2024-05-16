#include <Arduino.h>
#include <Wire.h>

//1. pin assignment
const byte kJoyXPin = A0;
const byte kJoyYPin = A1;
const byte kJoyBtnPin = 12;

//2. i2c wire
const byte kWireAddress = 9;
const byte kWireArrayLength = 3;

//3. timing
unsigned long us_current_time = millis();
unsigned long us_time_since_last_send = 0;
unsigned long us_last_debounce_time = 0;

//4. joystick and btn inputs
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


void readInputs(){
  i_joy_x = byte(analogRead(map(kJoyXPin,1,1024,1,255)));
  i_joy_y = byte(analogRead(map(kJoyYPin,1,1024,1,255)));
  Serial.print(digitalRead(kJoyBtnPin));
}

void readButton(){
  const byte kDebounceDelay = 50;
  int reading = digitalRead(kJoyBtnPin);

  if (reading != b_last_btn){
    us_last_debounce_time = us_current_time;
  }

  if ((us_current_time - us_last_debounce_time) > kDebounceDelay){
    if (reading != b_btn_state){
      b_btn_state = reading;
      if (b_btn_state == LOW){
        by_state++;
        keepStateWitihinRange();
      }
    }
  }
  b_last_btn = reading;
}

void keepStateWitihinRange(){
  if (by_state == kWireArrayLength) {
    by_state = 0;
  }
}

void sendData(){
  byte data[kWireArrayLength];
  data[0] = i_joy_x;
  data[1] = i_joy_y;
  data[2] = by_state;     
  Serial.println(data[2]);
  Wire.beginTransmission(kWireAddress);
  Wire.write(data, kWireArrayLength);
  Wire.endTransmission();
}

void collectAndSendData(){
  const int kSendingInterval = 300;
  if (us_current_time - us_time_since_last_send > kSendingInterval){
    readInputs();
    sendData();
    us_time_since_last_send = us_current_time;
  }
}

void loop() {
  us_current_time = millis();
  readButton();
  collectAndSendData();
}