
// digital
#define valve_1_pin 4
#define valve_2_pin 7
#define green_led_pin 5
#define gear_pin 6
#define red_led_pin 8
#define button_pin 9
#define btn_up_pin 3
#define btn_down_pin 2

// analog
#define flowrate_pin 0
// #define pressure_pin 1


#include <Arduino.h>
#include <U8g2lib.h>
#include <EEPROM.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif


// mini pro
#define clk 13
#define mosi 11
#define cs 10
#define dc 17
#define res 16


// micro lattepanda
// #define clk 3
// #define mosi 2
// #define ss 4
// #define dc 18
// #define res 9


// for testing()
#define min_rate 50
#define max_rate 150
#define pre_min_rate 300
#define pre_max_rate 400

// for blink_led
#define led_on_delay 100
#define led_off_delay 100
#define led_long_delay 1000

#define RATE_VOLTAGE_CORRECTION 0.986
#define RATE_VOLTAGE_OFFSET 1


// U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);    

// U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);  
// U8G2_SSD1309_128X64_NONAME2_2_4W_SW_SPI u8g2(U8G2_R3, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);
U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ clk, /* data=*/ mosi, /* cs=*/ cs, /* dc=*/ dc, /* reset=*/ res);


bool logs = false;

bool is_testing = false;

byte flow_pressure = 60;
int flow_rate = 60;

byte leds_state = 0;


float PFMV505_flow(const float voltage, const float offset){
  float x = voltage - offset;
  float y = (18.5 * (x * x)) + (45.7 * x);
  if (y < 0){
    y = 0;
  }
  return y * 1.04;
}

void enable(const int pin, const bool state){
  if (state){
    digitalWrite(pin, HIGH);
  }else{
    digitalWrite(pin, LOW);
  }
}

void flow_close(){
  digitalWrite(valve_1_pin, LOW);
  digitalWrite(valve_2_pin, LOW);
}

void flow_forward(){
  digitalWrite(valve_1_pin, LOW);
  digitalWrite(valve_2_pin, HIGH);
}

void flow_backward(){
  digitalWrite(valve_2_pin, LOW);
  digitalWrite(valve_1_pin, HIGH);
}

void set_flow_pressure(const byte st){
  analogWrite(gear_pin, st);
}


float get_flowrate(){
  float rate = 0.005 * RATE_VOLTAGE_CORRECTION * analogRead(flowrate_pin);
  Serial.print("Rate: ");
  Serial.print(rate);
  Serial.print("v,   ");
  return PFMV505_flow(rate, RATE_VOLTAGE_OFFSET);
}

bool is_equal(float a, float b, const byte precise){
  if (a < 0) a = 0;
  if (b < 0) b = 0;
  float precise_range = a*precise/100;
  if(abs(a-b) < precise_range){
    return true;
  }
  return false;
}


// float flow(const bool toward){
//   if(toward){
//     flow_forward();
//   }else{
//     flow_backward();
//   }
//   set_flow_pressure(flow_pressure);
//   delay(100);

//   int precise = 1; // in percent
//   float flowrate = 0;
//   int n = 4000;
//   int stable = 30;
//   while(n--){
//     float current_flowrate = get_flowrate();
//     Serial.print(current_flowrate);
//     Serial.print("sm3/m)");
//     Serial.println();
//     lcd_write(current_flowrate);
//     char buffer[10] = "Flowrate:";
//     u8g2.setFont(u8g2_font_5x8_t_cyrillic);
//     u8g2.drawStr(10, 10, buffer);
//     u8g2.sendBuffer();
//     if (is_equal(current_flowrate, flowrate, precise)){
//       if(!--stable){
//         Serial.print("stable when n = ");
//         Serial.println(n);
//         n = 0;
//       }
//     }
//     flowrate = current_flowrate;
//     read_buttons();
//     if(is_testing == false){
//       break;
//     }
//     delay(100);
//   }
//   set_flow_pressure(0);
//   flow_close();
//   Serial.println("The flowrate has been established!");
//   return flowrate;
// }


// byte testing(){
//   Serial.println("Testing is started...");
//   is_testing = true;
//   flow_close();
//   enable(red_led_pin, false);
//   enable(green_led_pin, false);
//   byte result = 0;
//   bool toward = false;
//   float last_flowrate = 0;
//   int stable = 3;
//   byte precise = 1; // in percent
//   float current_flowrate = 0;
//   int n = 100;
//   while(--n > 0){
//     current_flowrate = flow(toward);
//     if(is_equal(current_flowrate, last_flowrate, precise)){
//       Serial.println("Equal");
//       if(--stable < 0){
//         Serial.print("n = ");
//         Serial.println(n);
//         Serial.println("Testing done!");
//         n=0;
//       }
//     }else{
//       Serial.println("Not equal");
//     }
//     toward = !toward;
//     last_flowrate = current_flowrate;
//     if (is_testing == false){
//       break;
//     }
//   }
//   if (current_flowrate > pre_max_rate){
//     result = 4;
//   } else if (current_flowrate < pre_min_rate && current_flowrate > max_rate){
//     result = 3;
//   } else if (current_flowrate < min_rate){
//     result = 2;
//   } else {
//     result = 1;
//   }
//   return result;
// }


unsigned long int last_millis = 0;
int current_delay = 0;
bool state = false;
byte blink_counter = 0;

void blink_led(const byte pin, const byte count){
  unsigned long  int current_millis = millis();
  if(current_millis - last_millis > current_delay){
    last_millis = current_millis;
    state = !state;
    enable(pin, state);
    if (state){
      current_delay = led_on_delay;
    } else {
      if(++blink_counter == count){
        blink_counter = 0;
        current_delay = led_long_delay;
      } else {
        current_delay = led_off_delay;
      }
    }
    
  }
  
}



void lcd_write(int x){
  char buffer[4];
  itoa(x, buffer, 10);
  u8g2.clearBuffer();
  // u8g2.setDisplayRotation(U8G2_R2);
  // u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.setFont(u8g2_font_fub42_tr);
  u8g2.drawStr(10, 60, buffer);
  u8g2.sendBuffer();
}

void lcd_write3row(char buf1[24], char buf2[24]){
  // char buffer[10] = "Pressure:";
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_5x8_t_cyrillic);
  u8g2.drawStr(5, 15, buf1);
  u8g2.drawStr(5, 30, buf2);
  // u8g2.drawStr(5, 30, buf3);
  u8g2.sendBuffer();
}


void save_pressure(){
  EEPROM.write(0, flow_pressure);
  // EEPROM.commit();
}

bool is_flow_pressure_changed = false;

void set_new_flow_pressure(int value){
  if (value < 0){
    value = 0;
  } else if (value > 255){
    value = 255;
  }
  flow_pressure = value;
  is_flow_pressure_changed = true;

  lcd_write(flow_pressure);
  char buffer[10] = "Pressure:";
  u8g2.setFont(u8g2_font_5x8_t_cyrillic);
  u8g2.drawStr(10, 10, buffer);
  u8g2.sendBuffer();

}

void save_rate(){
  EEPROM.write(1, flow_rate);
  // EEPROM.commit();
}

bool is_flow_rate_changed = false;

void set_new_flow_rate(int value){
  if (value < 50){
    value = 50;
  } else if (value > 160){
    value = 160;
  }
  flow_rate = value;
  is_flow_rate_changed = true;

  lcd_write(flow_rate);
  char buffer[10] = "Flowrate:";
  u8g2.setFont(u8g2_font_5x8_t_cyrillic);
  u8g2.drawStr(10, 10, buffer);
  u8g2.sendBuffer();

}

unsigned long int last_time_pressed = 0;
bool is_skip = false;
void read_buttons(){
  
  bool btn_state = !digitalRead(button_pin);
  bool btn_up_state = !digitalRead(btn_up_pin);
  bool btn_down_state = !digitalRead(btn_down_pin);
  if(btn_state | btn_up_state | btn_down_state == true){
    unsigned long int current_time = millis();
    if(current_time - last_time_pressed < 10){
      return;
    }
    if(current_time - last_time_pressed < 200 && is_skip == true){
      return;
    }
    if(current_time - last_time_pressed > 150 && current_time - last_time_pressed < 500){
      return;
    }
    is_skip = true;
    is_testing = false;
    if (btn_up_state == true){
      set_new_flow_rate(flow_rate + 10);
    }
    if (btn_down_state == true){
      set_new_flow_rate(flow_rate - 10);
    }
  } else {
    last_time_pressed = millis();
    is_skip = false;
    if (is_flow_rate_changed == true){
      is_flow_rate_changed = false;
      save_rate();
    }
  }
}

#define rate_precision 1
#define flow_duration 10000//10000
#define good_count 20//20
// #define 

// byte start_pressure = 10;
// byte finish_pressure = 0;
byte loops_counter = 0;
bool flow_toward = false;
void dynamic_test(){
  flow_close();
  flow_pressure = 10;
  is_testing = true;
  byte err_counter = 0;
  while (is_testing){
    // flow_toward = !flow_toward;
    // if(flow_toward){
    //   flow_forward();
    // } else {
    //   flow_backward();
    // }
    float rate = step(flow_pressure);
    if(rate > 0){
      if (is_equal(rate, flow_rate, rate_precision)){
        loops_counter++;
      } else {
        loops_counter = 0;

        float k = flow_rate / rate;
        if (abs(k-1) > 0.2){
          flow_pressure *= 0.89*k;
        } else {
          if (rate > flow_rate){
            flow_pressure--;
          } else {
            flow_pressure++;
          }
        }
        
      
        // flow_pressure *= k;
        Serial.print("K = ");
        Serial.println(k);
        // char b1[24] = "k (scale):             ";
        // char b2[24];
        // dtostrf(k, 6, 2, b2);
        // char b3[24] = "Wait...                ";

        // lcd_write3row(b1, b2, b3);
        // delay(2000);
        
        
      }
    } else {
      err_counter++;
      flow_pressure++;
    }
    if(loops_counter >= good_count){
      set_new_flow_pressure(flow_pressure);
      is_testing = false;
      break;
    }
    // if (err_counter >= good_count){
    //   break;
    // }

  }
  loops_counter = 0;
  err_counter = 0;
  set_flow_pressure(0);
  flow_close();
}


float step(byte p){
  set_flow_pressure(p);
  flow_forward();
  wait(flow_duration);
  flow_backward();
  wait(flow_duration);
  return get_flowrate();
}

unsigned long int last_switch_millis = 0;
void wait(int d){
  last_switch_millis = millis();
  while(true){
    read_buttons();
    if(is_testing == false){
      break;
    }
    lcd_write(get_flowrate());
    // output flowrate, counter
    // char b1[24];
    // dtostrf(get_flowrate(), 24, 0, b1);
    // b1[0] = 'F'; b1[1] = 'l'; b1[2] = 'o'; b1[3] = 'w'; b1[4] = 'r'; b1[5] = 'a'; b1[6] = 't'; b1[7] = 'e'; b1[8] = ':';
    // char b2[24];
    // dtostrf(loops_counter, 24, 0, b2);
    // b2[0] = 'C'; b2[1] = 'o'; b2[2] = 'u'; b2[3] = 'n'; b2[4] = 't'; b2[5] = 'e'; b2[6] = 'r'; b2[7] = ':';
    // char b3[24] = "Wait...                ";

    // lcd_write3row(b2, b1);
    unsigned long int current_millis = millis();
    if(current_millis - last_switch_millis > d){
      break;
    }
  }
  
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setTimeout(1);
  Serial.print("Starting...      ");

  EEPROM.begin();
  int data = 0;
  data = EEPROM.read(0);
  flow_rate = EEPROM.read(1);
  if(data >= 0 && data <= 255){
    flow_pressure = data;
  }

  analogReference(INTERNAL);

  pinMode(valve_1_pin, OUTPUT);
  pinMode(valve_2_pin, OUTPUT);
  pinMode(flowrate_pin, OUTPUT);
  pinMode(gear_pin, OUTPUT);
  // pinMode(pressure_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(button_pin, INPUT);
  pinMode(btn_up_pin, INPUT);
  pinMode(btn_down_pin, INPUT);

  u8g2.begin();
  
  char buffer[12] = "Starting...";
//  itoa(x, buffer, 10);
  u8g2.clearBuffer();
  // u8g2.setDisplayRotation(U8G2_R2);
  // u8g2.setFont(u8g2_font_ncenB08_tr);
//  u8g2.setFont(u8g2_font_fub42_tr);
  // u8g2.setFont(u8g2_font_4x6_t_cyrillic);
  u8g2.setFont(u8g2_font_5x8_t_cyrillic);
  u8g2.drawStr(20, 20, buffer);
  u8g2.sendBuffer();


  digitalWrite(green_led_pin, HIGH);
  delay(200);
  digitalWrite(red_led_pin, HIGH);
  delay(500);
  digitalWrite(valve_1_pin, HIGH);
  delay(1000);
  digitalWrite(valve_1_pin, LOW);
  delay(500);
  digitalWrite(valve_2_pin, HIGH);
  delay(1000);
  digitalWrite(valve_2_pin, LOW);
  delay(500);
  digitalWrite(green_led_pin, LOW);
  digitalWrite(red_led_pin, LOW);


  char buf[6] = "Done!";
//  itoa(x, buffer, 10);
  u8g2.clearBuffer();
  // u8g2.setDisplayRotation(U8G2_R2);
  // u8g2.setFont(u8g2_font_ncenB08_tr);
//  u8g2.setFont(u8g2_font_fub42_tr);
  u8g2.setFont(u8g2_font_5x8_t_cyrillic);
  u8g2.drawStr(20, 20, buf);
  u8g2.sendBuffer();
//  
//  u8g2.clearBuffer();

  Serial.println("Done!");

  // char b1[24] = "Flowrate:............./";
  // dtostrf(get_flowrate(), 24, 0, b1);
  // b1[0] = 'F';
  // char b2[24] = "Counter:              /";
  // // dtostrf(loops_counter, 6, 0, b2);
  // char b3[24] = "Wait...               /";

  // lcd_write3row(b1, b2, b3);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  // that's for debugging
  if (Serial.available()){ 
    int ss = Serial.readString().toInt();
    if(ss > 255){
      if(ss == 300){
        digitalWrite(valve_1_pin, LOW);
      } else if (ss == 301){
        digitalWrite(valve_1_pin, HIGH);
      }else if (ss == 400){
        digitalWrite(valve_2_pin, LOW);
      }else if (ss == 401){
        digitalWrite(valve_2_pin, HIGH);
      } else if (ss == 500){
        digitalWrite(red_led_pin, LOW);
      }else if (ss == 501){
        digitalWrite(red_led_pin, HIGH);
      }else if (ss == 600){
        digitalWrite(green_led_pin, LOW);
      }else if (ss == 601){
        digitalWrite(green_led_pin, HIGH);
      }else if (ss == 700){
        logs = !logs;
      }else if (ss == 1000){
        // leds_state = testing();
      }else{
        Serial.println(is_equal(ss, 1000, 5));
      }
    } else {
      analogWrite(gear_pin, ss);
      if (ss == 0){
        u8g2.begin();
      }
    }
    lcd_write(ss);
  }

  bool btn_state = !digitalRead(button_pin);
  // Serial.println(btn_state);
  if(btn_state==true){
    // leds_state = testing();
    dynamic_test();
    // Serial.print("----");
  }

  if(logs){
    // int pressure = analogRead(pressure_pin);
  float rate = 0.005 * RATE_VOLTAGE_CORRECTION * analogRead(flowrate_pin);
  Serial.print("Flowrate: ");
  Serial.print(rate);
  Serial.print("v      (");
  Serial.print(PFMV505_flow(rate, RATE_VOLTAGE_OFFSET));
  Serial.print("sm3/m)");
  Serial.println();
  }
  if(leds_state == 0){ 
    enable(red_led_pin, false);
    enable(green_led_pin, false);
  } else if (leds_state == 1){
    enable(red_led_pin, false);
    enable(green_led_pin, true);
  } else {
    enable(red_led_pin, true);
    blink_led(green_led_pin, leds_state - 1);
  }
  read_buttons();
}
