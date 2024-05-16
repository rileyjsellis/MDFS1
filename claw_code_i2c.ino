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

void loop() {
    sendInputsData();
}

void sendInputsData(){
    unsigned long us_current_time = millis();
    unsigned long us_time_since_last_send;

    if (us_current_time - us_time_since_last_send > 300){
        readInputs();
        stateChangeDebounce();
        sendData();
        us_time_since_last_send = us_current_time;
    }
}

void readInputs(){
    joyX = analogRead(map(kJoyXPin,1,1024,1,255)); //reducing to send.
    joyY = analogRead(map(kJoyYPin,1,1024,1,255));
    Serial.print(digitalRead(kJoyBtnPin));
}

void stateChangeDebounce(){
    if (digitalRead(kJoyBtnPin) == LOW && millis() - us_button_timer > 1000) {
        by_state++;
        us_button_timer = millis();
        if (by_state == 3) {
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
    Wire.write(data, kWireArrayLength)
    Wire.endTransmission();
}