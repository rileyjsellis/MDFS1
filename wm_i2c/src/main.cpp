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
unsigned long us_button_timer = 0; //unsigned long saves us from lossy conversion errors

//4. joy and states inputs
byte i_joy_x = 0;
byte i_joy_y = 0;
byte by_state = 0;

void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(kJoyXPin, INPUT);
  pinMode(kJoyYPin, INPUT);
  pinMode(kJoyBtnPin, INPUT_PULLUP);
  digitalWrite(kJoyBtnPin, HIGH);
}


void readInputs(){
  i_joy_x = byte(analogRead(map(kJoyXPin,1,1024,1,255))); //reducing to send.
  i_joy_y = byte(analogRead(map(kJoyYPin,1,1024,1,255)));
  Serial.print(digitalRead(kJoyBtnPin));
}

void stateChangeDebounce(){
  const int i_delay = 1000;

  if (digitalRead(kJoyBtnPin) == LOW && millis() - us_button_timer > i_delay) {
    by_state++;
    us_button_timer = millis();
    if (by_state == kWireArrayLength) {
      by_state = 0;
    }
  }
}

void sendData(){
  byte data[kWireArrayLength];
  data[0] = i_joy_x;
  data[1] = i_joy_y;
  data[2] = by_state;     
  Serial.println(data[2]); //diagnostic println
  Wire.beginTransmission(kWireAddress);
  Wire.write(data, kWireArrayLength);
  Wire.endTransmission();
}

void sendInputsData(){
  unsigned long us_current_time = millis();
  unsigned long us_time_since_last_send;
  int sending_interval = 300;

  if (us_current_time - us_time_since_last_send > sending_interval){
    readInputs();
    stateChangeDebounce();
    sendData();
    us_time_since_last_send = us_current_time;
  }
}

void loop() {
  sendInputsData();
}