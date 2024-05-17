#include <Wire.h>
 
int joyX = 0;
int joyY = 0;
int state = 0;
int buttonTimer = 0;
 
void setup() {
  Serial.begin(9600);
  Wire.begin();
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  pinMode(12, INPUT);
  digitalWrite(12, HIGH);
 
}
 
void loop() { 
  
  joyX = analogRead(A0); // Read joystick X value (0-1023)
  joyY = analogRead(A1); // Read joystick Y value (0-1023)
  Serial.print(digitalRead(12));
  
  if (digitalRead(12) == LOW && millis() - buttonTimer > 1000) {
    state++;
    buttonTimer = millis();
    if (state == 3) {
      state = 0;
    }
  }
 
 
  // Prepare data to send
  byte data[5]; // Array to hold data bytes
  data[0] = highByte(joyX); // High byte of joyX
  data[1] = lowByte(joyX);  // Low byte of joyX
  data[2] = highByte(joyY); // High byte of joyY
  data[3] = lowByte(joyY);  // Low byte of joyY
  data[4] = state;           // State value (1 byte)
  Serial.print(data[4]);
  Serial.println();
  
  Wire.beginTransmission(9); // transmit to device #9
  Wire.write(data, 5);// sends x 
  Wire.endTransmission();    // stop transmitting
}