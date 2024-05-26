#include <Arduino.h>
#include <RF24.h>
#include <SPI.h>
#include <nRF24L01.h>

RF24 nrf(9, 8);  // CE, CSN previously:(9,8) //can be A0, 9
const byte linkAddress[6] =
    "link1";  // address through which two modules communicate.
const byte linkAddress2[6] = "link2";
int data[7];

// button
int button_count = 0;
int last_btn_count = 0;
int total_hits = 0;
int i_are_hits_up_to_date = 1;

// transmission data
unsigned long us_last_time_sent = 0;
int transmission_counter = 0;
bool b_is_transmitting = false;
bool b_data_shown = false;
int data_speed = 300;

//values to save and use
String state_desc[15];
byte state_selected;
int x_vals[15];
int y_vals[15];
int t_vals[15];
int turn_vals[15];

// old transmission sending data
// int left_wheel_percentage = 0;
// int right_wheel_percentage = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Starting");

  nrf.begin();
  nrf.openReadingPipe(0, linkAddress);
  nrf.openWritingPipe(linkAddress2);  // set the address
  nrf.startListening();               // Set nrf as receiver
}
///////////////////////////////////////////////////

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
      Serial.println("error: no data recieved from remote");
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
      i_are_hits_up_to_date = data[5];
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
      data[1] = x_vals[i];  // arm height
      data[2] = y_vals[i];
      data[3] = t_vals[i];          // diagnostic driving data
      data[4] = 1; //holder
      data[5] = total_hits;
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

void loop() {

  read_Data();

  send_Data();

  Display_Terminal_Data();

  transciever_Send_Or_Recieve();
}