#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include <RF24.h>
#include <SPI.h>
#include <Wire.h>
#include <nRF24L01.h>

// LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27, 16, 2);

RF24 nrf(9, 8);  // CE, CSN
const byte kLinkAddress[6] = "link1";
const byte kLinkAddress2[6] = "link2";
int data[7];
// char charVal[17];

// transmission data
unsigned long us_last_time_sent = 0;
int transmission_counter = 0;
bool b_is_transmitting = true;
bool b_data_shown = false;
int b_data_speed = 300;

const byte kJoyPins[4] = {A0, A1, A2, A3};
const int kJoyPinRange[4] = {400, 400, 16, 10};

const byte kJoyBtnPin = 2;

int i_joy_vals[4] = {0, 0, 0, 0};

// pushbutton
const byte kPushBtnPin = 4;
int last_push_counter = 0;

// joy+push
int i_button_counter[2] = {0, 0};
bool b_now_pressed[2] = {false, false};
bool b_prev_pressed[2] = {false, false};
bool b_debounce_progress[2] = {false, false};
unsigned long us_last_debounce_time = 0;
unsigned long us_debounce_delay = 50;

// display
String str_current_message_top = " ";
String str_current_message_bottom = " ";
String str_new_message_top = " ";
String str_new_message_bottom = " ";
int btn_count_display = 0;
int last_btn_count_display = 0;
unsigned long us_lcd_refresh = 0;
unsigned long us_lcd_last_update = 0;
int lcd_state = 0;
int last_lcd_state = 2;

// led
const int led = 6;
int led_brightness = 10;
int led_state = 0;
// colour of LED
enum colour { red = 5, green = 6 };

// recieving data
int arm_x = 0;
int arm_y = 0;

// old code to adjust if needed
int robot_internal_temp = 0;
int remaining_angle = 0;
int movement_state = 0;
int movement_state_a = 0;
String mov_state_1 = "";
String mov_state_a = "";
int total_hits = 0;

// hits transmission
int prev_total_hits = 0;
int i_are_sends_up_to_date = 1;

// timing
unsigned long us_current_time;
unsigned long us_last_joy_read = 0;

////////////////////////////////////////////////////
void setup() {
  // LCD
  lcd.init();       // initialize the lcd
  lcd.backlight();  // turn on backlight
  lcd.clear();      // clear the LCD, i.e. remove all characters

  //
  pinMode(kJoyBtnPin, INPUT_PULLUP);
  pinMode(kPushBtnPin, INPUT_PULLUP);
  pinMode(colour::red, OUTPUT);
  pinMode(colour::green, OUTPUT);
  Serial.begin(9600);
  nrf.begin();
  nrf.openWritingPipe(kLinkAddress);  // set the address
  nrf.openReadingPipe(0, kLinkAddress2);
  // nrf.setPALevel(RF24_PA_LOW);   //keep tx level low to stop psu noise, can
  // remove this but then put in decoupling caps
  //  Options are: RF24_PA_MIN=-18dBm, RF24_PA_LOW=-12dBm,  RF24_PA_HIGH=0dBm.
  nrf.setPALevel(RF24_PA_HIGH);
  nrf.stopListening();  // act as a transmitter
}

void loop() {
  us_current_time = millis();

  button_Pressed();

  transciever_Send_Or_Recieve();

  send_Data();

  read_Data();

  display_Terminal_Data();

  lcd_Status_Update();

  led_Status();
}

void readJoySticks() {
  for (int i = 0; i < 4; i++) {  // read each button
    int val = map(analogRead(kJoyPins[i]), 1, 1024, -5, 5);
    int read_pace = val;
    if (val < 0) {
      read_pace = val * (-1);
    }
    if (us_current_time >= (us_last_joy_read + 1000 / read_pace)) {
      us_last_joy_read = us_current_time;
      if (val >= 1) {
        i_joy_vals[i]++;
      } else if (val >= -1) {
        i_joy_vals[i]--;
      }
    }
  }
}

void transciever_Send_Or_Recieve() {
  if (b_is_transmitting == true) {
    if (transmission_counter >= 2) {
      transmission_counter = 0;
      nrf.startListening();
      b_is_transmitting = false;
    }
  }
  if (b_is_transmitting == false) {
    if (transmission_counter >= 1) {
      transmission_counter = 0;
      nrf.stopListening();
      b_is_transmitting = true;
      return;
    }
    if ((millis() - us_last_time_sent) > b_data_speed) {
      Serial.println("error: no data recieved from robot");
      transmission_counter = 3;
      led_state = 2;
      if (lcd_state != 0) {
        if (lcd_state >= 2) {
          last_lcd_state = lcd_state;
        }
        lcd_state = 1;  //"connection lost"
      }
    }
  }
}

void display_Terminal_Data() {
  if (b_data_shown == false) {
    if (b_is_transmitting == true) {
      Serial.print("t: ");
      Serial.print(int(data[0] / 100));
      Serial.print("x,");
      Serial.print(int(data[1] / 100));
      Serial.print("y, ");
      Serial.print(int(data[2] / 10.8));
      Serial.print("p, ");
      Serial.print(int(data[3] / 10.8));
      Serial.print("°, ");
      Serial.print(data[4]);
      Serial.print("b, ");
      Serial.print(data[6]);
      Serial.print(". tls:  ");
      Serial.println(String(millis()));
    }

    if (b_is_transmitting == false) {
      Serial.print("r: ");
      Serial.print(data[0]);
      Serial.print("°, ");
      Serial.print(data[1]);
      Serial.print("h, ");
      Serial.print(data[2]);
      Serial.print("°C, ");
      Serial.print(data[3]);
      Serial.print("l%, ");
      Serial.print(data[4]);
      Serial.print("r%, ");
      Serial.print(data[5]);
      Serial.print("b, ");
      Serial.print(data[6]);
      Serial.print(". tls:  ");
      Serial.println(String(millis()));
    }

    b_data_shown = true;
  }
}

void send_Data() {
  if (b_is_transmitting == true) {
    if ((millis() - us_last_time_sent) > b_data_speed) {
      transmission_counter = ++transmission_counter;

      data[0] = i_joy_vals[0];
      data[1] = i_joy_vals[1];
      data[2] = i_joy_vals[2];
      data[3] = i_joy_vals[3];
      data[4] = int(i_button_counter[1]);
      data[5] = i_are_sends_up_to_date;
      data[6] = transmission_counter;

      nrf.write(data, sizeof(data));  // spit out the data array
      us_last_time_sent = millis();
      b_data_shown = false;
    }
    if ((millis() - us_last_time_sent) > 200 && led_state == 1) {
      led_state = 0;
    }
  }
}

void read_Data() {
  if (b_is_transmitting == false) {
    if (nrf.available()) {
      nrf.read(&data, sizeof(data));

      arm_y = data[0];  // servo arms
      arm_x = data[1];
      // robot_internal_temp = data[2]; //internal temperature, works but not
      // enough pins
      remaining_angle = data[2];
      movement_state = data[3];  // wheel as percentage of 255
      movement_state_a = data[4];
      total_hits = data[5];
      transmission_counter = data[6];  // transmission
      led_state = 1;
      b_data_shown = false;
      process_Movement_State_Data();
      if (total_hits > prev_total_hits) {
        // prevents robot from hitting immediately if it's restarted but remote
        // stays on
        prev_total_hits = total_hits;
        i_are_sends_up_to_date = 1;
      }
      if (lcd_state <= 1) {          // gets lcd loop started again.
        lcd_state = last_lcd_state;  // continues from previous
      }
    }
  }
}

void process_Movement_State_Data() {
  switch (movement_state) {
    case 0:
      mov_state_1 = "stationary.";
      mov_state_a = "";
      break;
    case 1:
      mov_state_a = "towards heading.";
      if (movement_state_a == 1) {
        mov_state_1 = "turning left";
      }
      if (movement_state_a == 2) {
        mov_state_1 = "turning right";
      }
      break;
    case 2:
      mov_state_a = "towards heading.";
      if (movement_state_a == 1) {
        mov_state_1 = "veering left";
      }
      if (movement_state_a == 2) {
        mov_state_1 = "veering right";
      }
      if (movement_state_a == 3) {
        mov_state_1 = "forward";
        mov_state_a = " ";
      }
      if (movement_state_a == 4) {
        mov_state_1 = "slowing";
        mov_state_a = "no input";
      }
      break;
  }
}

void button_Pressed() {
  pressed(0, kPushBtnPin);
  pressed(1, kJoyBtnPin);
}

void pressed(int but, int pin) {
  b_now_pressed[but] =
      !digitalRead(pin);  // pressing results in 1., no press 10101
  // Serial.print(String(!digitalRead(pin)));

  if (!digitalRead(pin) == true) {
    b_now_pressed[but] = true;
  }

  if (b_now_pressed[but] != b_prev_pressed[but] &&
      b_debounce_progress[but] == false) {
    us_last_debounce_time = millis();
    b_debounce_progress[but] = true;
  }

  if ((millis() - us_last_debounce_time) > us_debounce_delay) {
    if (b_now_pressed[but] != b_prev_pressed[but]) {
      b_prev_pressed[but] = b_now_pressed[but];
      if (b_now_pressed[but] == true) {
        i_button_counter[but]++;
        b_debounce_progress[but] = false;
        i_are_sends_up_to_date = 0;
      }
    }
  }
}

void display(String message_top_line,
             String message_bottom_line) {  // if new message, print on LCD
  if (str_current_message_top != message_top_line ||
      str_current_message_bottom != message_bottom_line) {
    int textCentreTop = (16 - message_top_line.length()) / 2;
    int textCentreBottom = (16 - message_bottom_line.length()) / 2;
    lcd.clear();
    lcd.setCursor(textCentreTop, 0);
    lcd.print(message_top_line);
    lcd.setCursor(textCentreBottom, 1);
    lcd.print(message_bottom_line);
    str_current_message_top = message_top_line;
    str_current_message_bottom = message_bottom_line;
  }
}

void lcd_Status_Update() {
  if ((millis() - us_lcd_refresh) > 100) {
    String top_line;
    String bottom_line;
    if (i_button_counter[0] > last_push_counter &&
        lcd_state > 1) {  // state change displays
      lcd_state = ++lcd_state;
      if (lcd_state > 6) {
        lcd_state = 2;
      }
      last_push_counter = i_button_counter[0];
      us_lcd_last_update = millis();
    }
    if (lcd_state == 0) {
      display("awaiting     . .", "connection.  ---");
    }
    if (lcd_state == 1) {
      display("connection   ; ;", "lost.         ^ ");
    }
    if (lcd_state == 2) {  // most important data first
      /* //old, provides less diagnostic informatioon
       display("wheel s: " + String(left_wheel_percentage) + "%",
       "right wheel: " + String(right_wheel_percentage) + "%");
       */
      display(mov_state_1, mov_state_a);
    }
    if (lcd_state == 3) {
      lcd_state = 4;
      /*
      if (movement_state != 0){
        if (remaining_angle > 99){
          display("angle from      ", "heading: " + String(remaining_angle) + "'
      ");
        }
        if (remaining_angle > 9){
          display("angle from      ", "heading: " + String(remaining_angle) + "'
      ");
        }
        else{
          display("angle from      ", "heading: " + String(remaining_angle) + "'
      ");
        }
      }
      else{
        display("angle from      ", "heading: N/A    ");
      }
      */
    }
    if (lcd_state == 4) {  // arm status
      display("arm base: " + String(arm_y) + "'",
              "arm top: " + String(arm_x) + "'");
    }
    if (lcd_state == 5) {  // temp unavaliable, but will leave in
      display("additional setting space", "");
      // display("robot's internal", "temp: " + String(robot_internal_temp) +
      // "'C"); //for if pins avaliable
    }
    if (lcd_state == 6) {  // hit counter
      display("total given", "moves: " + String(total_hits));
    }
    us_lcd_refresh = millis();
  }
}

/*
OLD LCD STATUS UPDATE CODE, keep incase it's worth using again
if (joy_pos_vert > 700){
      str_new_message_bottom = "forward";
    }
    else if (joy_pos_horz > 564){
      str_new_message_bottom = "right";
    }
    else if (joy_pos_horz < 460) {
      str_new_message_bottom = "left";
    }
    else if (joy_pos_vert < 460){
      str_new_message_bottom = "backwards";
    }
    btn_count_display = buttonCount;
    if (btn_count_display != last_btn_count_display){
      us_time_since_hit_displayed = millis();
      last_btn_count_display = btn_count_display;
    }

    if ((millis() - us_time_since_hit_displayed) < 250){
      str_new_message_top = "*-*- HIT -*-*";
    }
    else if ((millis() - us_time_since_hit_displayed) < 500){
      str_new_message_top = "*-* HIT *-*";
    }
    else if ((millis() - us_time_since_hit_displayed) < 750){
      str_new_message_top = "-*- HIT -*-";
    }
    else if ((millis() - us_time_since_hit_displayed) < 1200){
      str_new_message_top = "*- HIT -*";
    }
    else if ((millis() - us_time_since_hit_displayed) < 1500){
      str_new_message_top = " ";
    }
  }
  display(str_new_message_top,str_new_message_bottom);
  lcdUpdated = true;
*/

void led_Status() {
  if (led_state == 0) {  // led off
    analogWrite(colour::red, 0);
    analogWrite(colour::green, 0);
  }
  if (led_state == 1) {  // data received, connection good.
    analogWrite(colour::red, 0);
    analogWrite(colour::green, led_brightness);
  }
  if (led_state ==
      2) {  // not data received, connection bad OR robot is turned off.
    analogWrite(colour::red, led_brightness);
    analogWrite(colour::green, 0);
  }
}