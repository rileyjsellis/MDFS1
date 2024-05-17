#include <Wire.h>
int joyX = 0;
int joyY = 0;
int state = 0;
 
const int enPin=8;
const int stepXPin = 2; //X.STEP
const int dirXPin = 5; // X.DIR
const int stepYPin = 3; //Y.STEP
const int dirYPin = 6; // Y.DIR
const int stepZPin = 4; //Z.STEP
const int dirZPin = 7; // Z.DIR
 
int stepPin = stepYPin;
int dirPin = dirYPin;
 
int stepsPerRev= 10;
int pulseWidthMicros = 100;   // microseconds
int millisBtwnSteps = 1000;
 
void receiveEvent(int numBytes) {
  if (numBytes >= 5) { // Expecting 5 bytes of data
    byte data[5];      // Array to store received data
    for (int i = 0; i < 5; i++) {
      data[i] = Wire.read(); // Read each byte of data
    }
 
    // Reconstruct the received values
    joyY = word(data[0], data[1]); // Combine high and low byte for joyX
    joyX = word(data[2], data[3]); // Combine high and low byte for joyY
    state = data[4]; 
    Serial.print(joyX);
    Serial.println(); 
  }
}
 
void setup() {
  pinMode(enPin, OUTPUT);
  digitalWrite(enPin, LOW);
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  
  Wire.begin(9);           // Join I2C bus as slave with address 9
  Wire.onReceive(receiveEvent); // Register receiveEvent function
  Serial.begin(9600);      // Initialize serial communication
}
void loop() {
 
 
    
  if (state == 0) {
       stepPin=stepYPin;
       dirPin=dirYPin;
    
    if (joyX < 490) {
      digitalWrite(dirPin, LOW); //Changes the rotations direction
      //stepsPerRev = map(joyX, 0, 499, 1, 10); 
      for (int i = 0; i < stepsPerRev; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(millisBtwnSteps); 
      }
    } else if (joyX > 510) {
      digitalWrite(dirPin, HIGH); //Changes the rotations direction
      //stepsPerRev = map(joyX, 503, 1023, 1, 10); 
      for (int i = 0; i < stepsPerRev; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(millisBtwnSteps);
      }
    } else {
      digitalWrite(stepPin, LOW);
    }
    
  } else if (state == 1) {
    
    stepPin=stepXPin;
    dirPin=dirXPin;
    
     if (joyX < 495) {
      digitalWrite(dirPin, LOW); //Changes the rotations direction
      //stepsPerRev = map(joyX, 0, 499, 1, 10); 
      for (int i = 0; i < stepsPerRev; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(millisBtwnSteps); 
      }
    } else if (joyX > 505) {
      digitalWrite(dirPin, HIGH); //Changes the rotations direction
      //stepsPerRev = map(joyX, 503, 1023, 1, 10); 
      for (int i = 0; i < stepsPerRev; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(millisBtwnSteps);
      }
    } else {
      digitalWrite(stepPin, LOW);
    }
      
    stepPin=stepZPin;
    dirPin=dirZPin;
    
     if (joyX < 495) {
      digitalWrite(dirPin, LOW); //Changes the rotations direction
      //stepsPerRev = map(joyX, 0, 499, 1, 10); 
      for (int i = 0; i < stepsPerRev; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(millisBtwnSteps); 
      }
    } else if (joyX > 505) {
      digitalWrite(dirPin, HIGH); //Changes the rotations direction
      //stepsPerRev = map(joyX, 503, 1023, 1, 10); 
      for (int i = 0; i < stepsPerRev; i++) {
        digitalWrite(stepPin, HIGH);
        delayMicroseconds(pulseWidthMicros);
        digitalWrite(stepPin, LOW);
        delayMicroseconds(millisBtwnSteps);
      }
    } else {
      digitalWrite(stepPin, LOW);
    }
    
  }
}