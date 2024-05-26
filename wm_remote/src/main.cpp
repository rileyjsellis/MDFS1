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

// joystick values to send
int i_joy_vals[4] = {0, 0, 0, 0};
const byte kJoyPins[4] = {A1, A0, A3, A2};
const byte kJoyBtnPin = 2;

// important state values;
const int kPodNut = 6;
const int kTree300[] = {200, 300};
const int kTree150[] = {150, 150};
const int kGround300[] = {300, kPodNut};
const int kGround200[] = {200, kPodNut};  // two of these.
const int kGround150[] = {150, kPodNut};

// initial testing values:
int currently_testing = 1;
String state_desc[15] = {"waiting for switch", "reverse turn", "col. pod 1",
                       "to tree 1",          "col. pod 2",   "to pod 3",
                       "col. pod 3",         "reverse turn", "col. pod 4",
                       "to tree 2",          "col. pod 5",   "to pod 6",
                       "col. pod 6",         "reverse turn", "DEPOSIT POS."};
const int kTurnEstimate = 300;
int x_vals[15] = {0, //x is used for some values as turn.
                  kTurnEstimate,
                  kGround150[0],
                  0,
                  kTree150[0],
                  0,
                  kGround200[0],
                  kTurnEstimate,
                  kGround300[0],
                  0,
                  kTree300[0],
                  0,
                  kGround200[0],
                  kTurnEstimate,
                  -10};
int y_vals[15] = {0,
                0,
                kGround150[1],  // also turn intensity vals
                0,
                kTree150[1],
                0,
                kGround200[1],
                0,
                kGround300[1],
                0,
                kTree300[1],
                0,
                kGround200[1],
                0,
                100};
int t_vals[15] = {
    0, 2000, 0, 1000, 0, 1000, 0, 3000, 0, 1000, 0, 2000, 0, 1500,0};

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
byte lcd_state = 0;

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

void readJoySticks() {
  if (us_current_time >= (us_last_joy_read + 500)) {
    us_last_joy_read = us_current_time;
    bool b_has_increased = false;
    for (int i = 0; i < 4; i++) {
      int val = map(analogRead(kJoyPins[i]), 1, 1024, -5, 5);
      if (i == 0 || i == 1) {  // offset the input joystick
        val = val * (-1);
      }
      if (val >= 2 && b_has_increased == false) {
        b_has_increased = true;
        if (i == 0) {
          x_vals[currently_testing]++;
        } else if (i == 1) {
          y_vals[currently_testing]++;
        } else if (i == 2) {
          t_vals[currently_testing]++;
        } else if (i == 3) {
          currently_testing ++;
          if (currently_testing > 15) {
            currently_testing = 0;
          }
        }
        break;
      } else if (val <= -2 && b_has_increased == false) {
        b_has_increased = true;
        Serial.println("r-");
        if (i == 0) {
          x_vals[currently_testing]--;
        } else if (i == 1) {
          y_vals[currently_testing]--;
        } else if (i == 2) {
          t_vals[currently_testing]--;
        } else if (i == 3) {
         currently_testing--;
         Serial.println(currently_testing);
         if (currently_testing < 0){
          currently_testing = 15;
         }
         Serial.println(currently_testing);
        }
        break;
      }
      // Serial.println(String(String(i) + " " + String(i_joy_vals[i]) + "
      // changed.");
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
    if (float(us_current_time - us_last_time_sent) >= b_data_speed) {
      // Serial.println("error: no data recieved from robot");
      transmission_counter = 3;
      led_state = 2;
      if (float(us_current_time - us_last_time_sent) >= (b_data_speed * 5)) {
        // if (lcd_state == 2) {  // 0, awaiting connection
        //   lcd_state = 1;       //"connection lost"
        // }
      }
    }
  }
}

void display_Terminal_Data() {
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
      Serial.println(String(us_current_time));
    }

    if (b_is_transmitting == false) {
      Serial.print("rcve: ");
      Serial.print(data[0] + ",");
      Serial.print(data[1] + " x, ");
      Serial.print(data[2] + " y, ");
      Serial.print(data[3] + "millis, ");
      Serial.print(data[4] + "t rad, ");
      Serial.print("r%, ");
      Serial.print(data[5]);
      Serial.print("b, ");
      Serial.print(data[6]);
      Serial.print("tls. ");
      Serial.println(String(us_current_time));
    }

    b_data_shown = true;
  }
}

void send_Data() {
  if (b_is_transmitting == true) {
    if ((us_current_time - us_last_time_sent) > b_data_speed) {
      transmission_counter++;

      int i = currently_testing;

      data[0] = currently_testing;
      data[1] = x_vals[i];                 // x pos
      data[2] = y_vals[i];                 // y pos
      data[3] = t_vals[i];                 // state
      data[4] = int(i_button_counter[1]);  // run test with values.
      data[5] = i_are_sends_up_to_date;
      data[6] = transmission_counter;

      nrf.write(data, sizeof(data));  // spit out the data array
      us_last_time_sent = us_current_time;
      b_data_shown = false;
    }
    if ((us_current_time - us_last_time_sent) > 200 && led_state == 1) {
      led_state = 0;
    }
  }
}

void read_Data() {
  if (b_is_transmitting == false) {
    if (nrf.available()) {
      nrf.read(&data, sizeof(data));

      // does nothing with data coming through beyond reading it.
      total_hits = data[5];
      transmission_counter = data[6];  // transmission
      led_state = 1;
      b_data_shown = false;
      if (total_hits > prev_total_hits) {
        // prevents robot from hitting immediately if it's restarted but remote
        // stays on
        prev_total_hits = total_hits;
        i_are_sends_up_to_date = 1;
      } else {
      }
    }
  }
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
    us_last_debounce_time = us_current_time;
    b_debounce_progress[but] = true;
  }

  if ((us_current_time - us_last_debounce_time) > us_debounce_delay) {
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

void button_Pressed() {
  pressed(0, kPushBtnPin);
  pressed(1, kJoyBtnPin);
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
  if ((us_current_time - us_lcd_refresh) > 250) {
    /*
    if (lcd_state == 0) {  // if
      display("awaiting     . .", "connection.  ---");
    }
    if (lcd_state == 1) {  // if
      display("connection   ; ;", "lost.         ^ ");
    }
    */
    Serial.println(String(currently_testing));
    int i = currently_testing;
    if (i == 0) {  // waiting
      display(String(i) + ": " + state_desc[i],
              "no change.");
    } else if (i == 1 || i == 7 || i == 13) {
      display(String(i) + ": " + state_desc[i],
              "t: " + String(t_vals[i]) + ", r: " + String(x_vals[i]));
      }
    else if (i == 3 || i == 5 || i == 9 || i == 11) {
      display(String(i) + ": " + state_desc[i],
              "duration: " + String(t_vals[i]));
    } else if (i == 2 || i == 4 || i == 6 || i == 8 || i == 10 || i == 12 ||
               i == 14) {
      display(String(i) + ": " + state_desc[i],
              "x: " + String(x_vals[i]) + ", y: " + String(y_vals[i]));
      }
    us_lcd_refresh = us_current_time;
  }
}

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

void loop() {
  us_current_time = millis();

  readJoySticks();

  button_Pressed();

  transciever_Send_Or_Recieve();

  send_Data();

  read_Data();

  // display_Terminal_Data();

  lcd_Status_Update();

  led_Status();
}