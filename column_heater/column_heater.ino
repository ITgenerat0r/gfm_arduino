#include <Arduino.h>
#include <U8g2lib.h>
#include <EEPROM.h>

#include <stm32_def.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif

#include <OLEDHelper.h>

#include "pid.h"
#include <stdio.h>


// digital

#define RED_LED_PIN PB3
#define GREEN_LED_PIN PB4
#define BTN_START_PIN PB10
#define BTN_UP_PIN PB1
#define BTN_DOWN_PIN PB11
#define BTN_MODE_PIN PB0

// analog
#define FLOWRATE_PIN PA3
// #define pressure_pin 2



// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif



// display ssd1309
#define CLK_PIN PB13
#define MOSI_PIN PB15
#define CS_PIN PB12
#define DC_PIN PB9
#define RES_PIN PB8


// temperature device


// micro lattepanda
// #define CLK_PIN 3
// #define MOSI_PIN 2
// #define ss 4
// #define DC_PIN 18
// #define RES_PIN 9



// for blink_led
#define LED_ON_DELAY 100
#define LED_OFF_DELAY 100
#define LED_LONG_DELAY 1000

#define RATE_VOLTAGE_CORRECTION 1.124 // 0.986
#define RATE_VOLTAGE_OFFSET 1

#define PRESSURE_CORRECTION 0.000685156

// other
#define MAX_TEMPERATURE_PRESET 300

#define HEATER_PIN PA15


// U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);    

// U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);  
// U8G2_SSD1309_128X64_NONAME2_2_4W_SW_SPI u8g2(U8G2_R3, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);
U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(
  U8G2_R2, 
  /* clock=*/ CLK_PIN, 
  /* data=*/ MOSI_PIN, 
  /* cs=*/ CS_PIN, 
  /* dc=*/ DC_PIN, 
  /* reset=*/ RES_PIN
);

OLEDHelper oled(u8g2);


bool logs = false;


// blink_led()
byte leds_state = 0;
unsigned long last_millis = 0;
int current_delay = 0;
bool state = false;
byte blink_counter = 0;


// read_buttons()
unsigned long last_time_pressed = 0;
bool is_skip = false;


// display
char result_buf[25];
char info_buf[25];
char current_temp_buf[25];


// wait()
unsigned long last_switch_millis = 0;



// other
int target_temperature = 0;
int temperature = 0;

bool is_heating = false;
bool is_temperature_changed = false;









// float PFMV505_flow(const float voltage, const float offset){
//   float x = voltage - offset;
//   float y = (18.5 * (x * x)) + (45.7 * x);
//   if (y < 0){
//     y = 0;
//   }
//   return y * 1.04;
// }

void enable(const int pin, const bool state){
  if (state){
    digitalWrite(pin, HIGH);
  }else{
    digitalWrite(pin, LOW);
  }
}






bool is_equal(float a, float b, const byte precise){
  if (a < 0) a = 0;
  if (b < 0) b = 0;
  float precise_range = a*precise/100;
  if(abs(a-b) <= precise_range){
    return true;
  }
  return false;
}

bool is_equal(float a, float b, const byte precise, byte tw){
  if (a < 0) a = 0;
  if (b < 0) b = 0;
  float precise_range = a*precise/100;
  if(tw == 0){
    if(abs(a-b) <= precise_range){
      return true;
    }
  } else if (tw == 1){
    if((a > b - precise_range) && (a < b)){
      return true;
    }
  } else if (tw == 2){
    if((a > b) && (a < b + precise_range)){
      return true;
    }
  }
  
  return false;
}







void blink_led(const byte pin, const byte count){
  unsigned long  current_millis = millis();
  if(current_millis - last_millis > current_delay){
    last_millis = current_millis;
    state = !state;
    enable(pin, state);
    if (state){
      current_delay = LED_ON_DELAY;
    } else {
      if(++blink_counter == count){
        blink_counter = 0;
        current_delay = LED_LONG_DELAY;
      } else {
        current_delay = LED_OFF_DELAY;
      }
    }
    
  }
  
}

void heat_on(){
  digitalWrite(HEATER_PIN, LOW);
}

void heat_off(){
  digitalWrite(HEATER_PIN, HIGH);
}

void save_temperature(){
  EEPROM.write(0, target_temperature);
  // EEPROM.commit();
}


void set_new_temperature(int value){
  if (value < 0){
    value = 0;
  } else if (value > MAX_TEMPERATURE_PRESET){
    value = MAX_TEMPERATURE_PRESET;
  }
  target_temperature = value;
  is_temperature_changed = true;
  snprintf(info_buf, sizeof(info_buf), "Target: %d", target_temperature);
  // update_monitor();
}


void read_buttons(){
  
  bool btn_state = digitalRead(BTN_START_PIN);
  bool btn_up_state = digitalRead(BTN_UP_PIN);
  bool btn_down_state = digitalRead(BTN_DOWN_PIN);
  bool btn_mode_state = digitalRead(BTN_MODE_PIN);
  if(btn_state | btn_up_state | btn_down_state | btn_mode_state == true){
    unsigned long current_time = millis();

    // debounce filter
    if(current_time - last_time_pressed < 10){
      return;
    }

    // sckip multiple increment if short pressed
    if(current_time - last_time_pressed < 200 && is_skip == true){
      return;
    }

    // for long pressed
    if(current_time - last_time_pressed > 150 && current_time - last_time_pressed < 500){
      return;
    }
    is_skip = true;
    // is_testing = false;
    
    if (btn_up_state == true){
      set_new_temperature(target_temperature+1);
      snprintf(result_buf, sizeof(result_buf), "UP btn.");
    }
    if (btn_down_state == true){
      set_new_temperature(target_temperature-1);
      snprintf(result_buf, sizeof(result_buf), "DOWN btn.");
    }
    
    
    
    if (btn_state == true){
      // ok
      // run heat
      heat_on();
      snprintf(result_buf, sizeof(result_buf), "OK btn.");
    }
    if (btn_mode_state == true){
      // back 
      // stop heat
      heat_off();
      snprintf(result_buf, sizeof(result_buf), "BACK btn.");
    }
  } else {
    snprintf(result_buf, sizeof(result_buf), "");
    last_time_pressed = millis();
    is_skip = false;
    if (is_temperature_changed == true){
      is_temperature_changed = false;
      save_temperature();
    }
  }
}


float get_temperature(){
  // get T


  char _low_buffer[16];
  dtostrf(temperature, 4, 2, _low_buffer);
  snprintf(current_temp_buf, 25, "Temperature: ", _low_buffer);
  return temperature;
}


void update_monitor(){
  oled.clear();
  // oled.drawMode("Target: ", target_temperature);
  // if(md_full){
  //   oled.drawMode("Full", flow_rate_full);
  // } else {
  //   oled.drawMode("Single", flow_rate_single);
  // }
  // oled.drawRate(get_flowrate());
  // oled.drawPressure(temperature);
  get_temperature();

  const char* t_pointer = current_temp_buf;
  oled.drawRow(2, t_pointer);

  const char* result_pointer = result_buf;
  oled.drawRow(3, result_pointer);

  const char* info_pointer = info_buf;
  oled.drawRow(1, info_pointer);

  oled.update();
}



void wait(int d){
  last_switch_millis = millis();
  // unsigned long last_output = millis();
  // snprintf(info_buf, sizeof(info_buf), "wait... ");
  while(is_heating){
    update_monitor();
    read_buttons();
    // lcd_write(get_flowrate());
    unsigned long current_millis = millis();
    if(current_millis - last_switch_millis > d){
      break;
    }
    // snprintf(info_buf, sizeof(info_buf), "wait... %d", current_millis - last_switch_millis);
    // if(current_millis - last_output > 5000){
    //   last_output = millis();
    //   update_monitor();
    // }
  }
  
}

void wait_t(int d){
  last_switch_millis = millis();
  // unsigned long last_output = millis();
  // snprintf(info_buf, sizeof(info_buf), "wait... ");
  while(is_heating){
    update_monitor();
    read_buttons();
    // lcd_write(get_flowrate());
    unsigned long current_millis = millis();
    if(current_millis - last_switch_millis > d){
      break;
    }
    // snprintf(info_buf, sizeof(info_buf), "wait... %d", current_millis - last_switch_millis);
    // if(current_millis - last_output > 5000){
    //   last_output = millis();
    //   update_monitor();
    // }
  }
  
}










void setup() {
  // put your setup code here, to run once:
  // Serial.begin(9600);
  // Serial.setTimeout(1);
  // Serial.print("Starting...      ");

  // Включаем тактирование порта B
  // RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  // // Настройка PB5 как вход без pull-up/pull-down
  // GPIOB->CRL &= ~(0xF << (5 * 4));  // Очистить биты настройки
  // GPIOB->CRL |= (0x00 << (5 * 4));  // Input floating

  EEPROM.begin();
  int data = 0;
  data = EEPROM.read(0);
  if(data >= 0 && data <= MAX_TEMPERATURE_PRESET){
    target_temperature = data;
  }

  // analogReference(INTERNAL);




  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BTN_START_PIN, INPUT);
  pinMode(BTN_UP_PIN, INPUT);
  pinMode(BTN_DOWN_PIN, INPUT);
  pinMode(BTN_MODE_PIN, INPUT);

  pinMode(HEATER_PIN, OUTPUT);
  heat_off();

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // char buffer[12] = "Starting...";
//  itoa(x, buffer, 10);
  oled.clear();
  oled.drawRow(1, "Starting...");
  oled.drawRow(2, "Version 1.0");
  oled.update();


  snprintf(info_buf, sizeof(info_buf), "Target: %d", target_temperature);
  snprintf(result_buf, sizeof(result_buf), "Done!");
  
  


  // digitalWrite(GREEN_LED_PIN, HIGH);
  // delay(200);
  // digitalWrite(RED_LED_PIN, HIGH);
  // delay(500);
  // digitalWrite(VALVE_1_PIN, HIGH);
  // delay(1000);
  // digitalWrite(VALVE_1_PIN, LOW);
  // delay(500);
  // digitalWrite(VALVE_2_PIN, HIGH);
  // delay(1000);
  // digitalWrite(VALVE_2_PIN, LOW);
  // delay(500);
  // digitalWrite(GREEN_LED_PIN, LOW);
  // digitalWrite(RED_LED_PIN, LOW);


  // oled.clear();
  oled.drawRow(3, "Done!");
  oled.update();
//  
//  u8g2.clearBuffer();

  // Serial.println("Done!");

  // char b1[24] = "Flowrate:............./";
  // dtostrf(get_flowrate(), 24, 0, b1);
  // b1[0] = 'F';
  // char b2[24] = "Counter:              /";
  // // dtostrf(loops_counter, 6, 0, b2);
  // char b3[24] = "Wait...               /";

  // lcd_write3row(b1, b2, b3);
}

void loop() {
  
  if(logs){
    // int pressure = analogRead(pressure_pin);
  // Serial.print("Flowrate: ");
  // Serial.print(rate);
  // Serial.print("v      (");
  // Serial.print(PFMV505_flow(rate, RATE_VOLTAGE_OFFSET));
  // Serial.print("sm3/m)");
  // Serial.println();
  }
  update_monitor();
  read_buttons();
}
