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
#define VALVE_1_PIN PA8
#define VALVE_2_PIN PA15
#define RED_LED_PIN PB3
#define GREEN_LED_PIN PB4
#define GEAR_PIN PA1
#define BTN_START_PIN PB6
#define BTN_UP_PIN PB7
#define BTN_DOWN_PIN PB9
#define BTN_MODE_PIN PB8

// analog
#define FLOWRATE_PIN PA3
// #define pressure_pin 2



// #ifdef U8X8_HAVE_HW_I2C
// #include <Wire.h>
// #endif


// mini pro
#define CLK_PIN PA5
#define MOSI_PIN PA7
#define CS_PIN PA4
#define DC_PIN PB0
#define RES_PIN PB1


// micro lattepanda
// #define CLK_PIN 3
// #define MOSI_PIN 2
// #define ss 4
// #define DC_PIN 18
// #define RES_PIN 9


// for testing()
#define MIN_RATE 50
#define MAX_RATE 150
#define PRE_MIN_RATE 300
#define PRE_MAX_RATE 400

// for blink_led
#define LED_ON_DELAY 100
#define LED_OFF_DELAY 100
#define LED_LONG_DELAY 1000

#define RATE_VOLTAGE_CORRECTION 1.124 // 0.986
#define RATE_VOLTAGE_OFFSET 1

#define PRESSURE_CORRECTION 0.000685156

// other
#define RATE_PRECISION 3
#define PRESSURE_PRECISION 3
#define FINAL_RATE_PRECISION 5
#define FLOW_DURATION 10000//10000
#define GOOD_COUNT 20//20
#define SLOW_I_DELAY_BEFORE_CHANGE 6000
#define MIN_SLOW_I_RANGE 2
#define MAX_SLOW_I_RANGE 10
// #define 

#define FINAL_LOOPS 20





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

bool is_testing = false;
bool is_d_testing = false;


// blink_led()
byte leds_state = 0;
unsigned long last_millis = 0;
int current_delay = 0;
bool state = false;
byte blink_counter = 0;


// set_new_flow_pressure()
bool is_flow_pressure_changed = false;
int flow_pressure = 60;
float current_pressure = 0;

const int PRESSURE_PIN = A0;
const int NUM_SAMPLES = 16;        // для усреднения
float alpha = 0.1;                 // коэффициент экспоненциального фильтра
float filteredPressure = 0.0;      // отфильтрованное давление (в Па)

// set_new_flow_rate()
bool is_flow_rate_changed = false;
int flow_rate_full = 60;
int flow_rate_single = 70;

// int flow_rate_table[] = {0};
byte flow_pos = 0;
float current_flowrate = 0;
float flow_res[FINAL_LOOPS] = { 0.0 };





// read_buttons()
unsigned long last_time_pressed = 0;
bool is_skip = false;


// dynamic_test()
// byte start_pressure = 10;
// byte finish_pressure = 0;
bool flow_toward = false;
int slow_i_delay = 1000;
byte slow_i_range = 5;


// wait()
unsigned long last_switch_millis = 0;



bool md_full = true; // mode
bool static_mode = false;
bool is_flow_ready = false;
bool dynamic_mode = false;
char result_buf[25];
char info_buf[25];









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
  digitalWrite(VALVE_1_PIN, LOW);
  digitalWrite(VALVE_2_PIN, LOW);
}

void flow_forward(){
  digitalWrite(VALVE_1_PIN, LOW);
  digitalWrite(VALVE_2_PIN, HIGH);
}

void flow_backward(){
  digitalWrite(VALVE_2_PIN, LOW);
  digitalWrite(VALVE_1_PIN, HIGH);
}

void set_flow_pressure(const int st){
  analogWrite(GEAR_PIN, st);
  current_pressure = st*PRESSURE_CORRECTION;
}


float get_flowrate(){
  // float u = 0.5 * RATE_VOLTAGE_CORRECTION * analogRead(FLOWRATE_PIN);
  // float rate = PFMV505_flow(u, RATE_VOLTAGE_OFFSET);
  
  // Шаг 1: усреднение 16 отсчётов
  long adcSum = 0;
  for (int i = 0; i < NUM_SAMPLES; i++) {
    adcSum += analogRead(FLOWRATE_PIN);
    delay(1);  // стабилизация
    read_buttons();
  }
  int x = adcSum / NUM_SAMPLES;

  // flow_rate_table[flow_pos] = x;
  // if (++flow_pos >= sizeof(flow_rate_table)){
  //   flow_pos = 0;
  // }
  // long int sum_flow = 0;
  // for (byte i = 0; i < sizeof(flow_rate_table); i++){
  //   sum_flow += flow_rate_table[i];
  // }
  // sum_flow /= sizeof(flow_rate_table);


  float u = 5.0 * x * (3.3 / 4095.0) * RATE_VOLTAGE_CORRECTION;
  float rate = PFMV505_flow(u, RATE_VOLTAGE_OFFSET);

  current_flowrate = alpha * rate + (1 - alpha) * current_flowrate;

  // flow_pos++;
  // if (flow_pos >= sizeof(flow_rate_table)){
  //   flow_pos = 0;
  // }
  // flow_rate_table[flow_pos] = rate;
  // float sum_flow = 0.0;
  // for (byte i = 0; i < sizeof(flow_rate_table); i++){
  //   sum_flow += flow_rate_table[i]/sizeof(flow_rate_table);
  // }
  
  // current_flowrate += rate;
  // current_flowrate /= 2;
  // Serial.print("Flowrate: ");
  // Serial.println(current_flowrate);
  return current_flowrate;
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

void save_pressure(){
  EEPROM.write(0, flow_pressure);
  // EEPROM.commit();
}

void save_rate(){
  EEPROM.write(1, flow_rate_full);
  EEPROM.write(2, flow_rate_single);
  // EEPROM.commit();
}


void set_new_flow_pressure(int value){
  if (value < 0){
    value = 0;
  } else if (value > 4095){
    value = 4095;
  }
  flow_pressure = value;
  is_flow_pressure_changed = true;
}

void set_new_flow_rate(int value){
  

  
  flow_rate_full += value;
  // fix upper step rate
  if(flow_rate_full == 140) flow_rate_full = 150;
  if(flow_rate_full == 130) flow_rate_full = 120;
  // check borders
  if (flow_rate_full > 150) flow_rate_full = 150;
  if (flow_rate_full < 60) flow_rate_full = 60;
  
  is_flow_rate_changed = true;
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
    if(dynamic_mode == false){
      if (btn_up_state == true){
        set_new_flow_rate(20);
        slow_i_delay = 1000;
      }
      if (btn_down_state == true){
        set_new_flow_rate(-20);
        slow_i_delay = 1000;
      }
    } 
    // if (is_d_testing){
    //   if(btn_up_state == true){
    //     flow_pressure++;
    //   } 
    //   if (btn_down_state == true){
    //     flow_pressure--;
    //   }
    //   set_flow_pressure(flow_pressure);
    //   snprintf(info_buf, sizeof(info_buf), "dynamic, AO=%d", flow_pressure);
    // }
    
    if (btn_state == true){
      dynamic_mode = false;
      if(static_mode == true){
        is_testing = false;
      } else {
        static_test();
      }
    }
    if (btn_mode_state == true){
      if(dynamic_mode == true && is_d_testing){
        is_d_testing = false;
        dynamic_mode = false;
      } else {
        if(is_flow_ready == true){
          dynamic_test();
        }
      }
      
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


void update_monitor(){
  oled.clear();
  oled.drawMode("Target: ", flow_rate_full);
  // if(md_full){
  //   oled.drawMode("Full", flow_rate_full);
  // } else {
  //   oled.drawMode("Single", flow_rate_single);
  // }
  oled.drawRate(get_flowrate());
  oled.drawPressure(current_pressure);

  const char* result_pointer = result_buf;
  oled.drawRow(4, result_pointer);

  const char* info_pointer = info_buf;
  oled.drawRow(5, info_pointer);

  oled.update();
}


// void update_monitor(const char* data){
//   oled.clear();
//   if(md_full){
//     oled.drawMode("Full", flow_rate_full);
//   } else {
//     oled.drawMode("Single", flow_rate_single);
//   }
//   oled.drawRate(get_flowrate());
//   oled.drawPressure(current_pressure);
//   oled.drawResult(data);

//   oled.update();
// }

// void update_monitor2(float rate, float pressure, const char* result){
//   Serial.println("UPDATE 2");
//   oled.clear();
//   if(md_full){
//     oled.drawMode("Full", flow_rate_full);
//   } else {
//     oled.drawMode("Single", flow_rate_single);
//   }
//   oled.drawRate(rate);
//   oled.drawPressure(pressure);
//   oled.drawResult(result);

//   oled.update();
// }




// void dynamic_test(float flow_rate){
  // flow_close();
  // flow_pressure = 10;
  // is_testing = true;
  // byte err_counter = 0;
  // while (is_testing){
  //   // flow_toward = !flow_toward;
  //   // if(flow_toward){
  //   //   flow_forward();
  //   // } else {
  //   //   flow_backward();
  //   // }
  //   float rate = step(flow_pressure);
  //   if(rate > 0){
  //     if (is_equal(rate, flow_rate, RATE_PRECISION)){
  //       loops_counter++;
  //     } else {
  //       loops_counter = 0;

  //       float k = flow_rate / rate;
  //       if (abs(k-1) > 0.2){
  //         flow_pressure *= 0.89*k;
  //       } else {
  //         if (rate > flow_rate){
  //           flow_pressure--;
  //         } else {
  //           flow_pressure++;
  //         }
  //       }
        
      
  //       // flow_pressure *= k;
  //       Serial.print("K = ");
  //       Serial.println(k);
  //       // char b1[24] = "k (scale):             ";
  //       // char b2[24];
  //       // dtostrf(k, 6, 2, b2);
  //       // char b3[24] = "Wait...                ";

  //       // lcd_write3row(b1, b2, b3);
  //       // delay(2000);
        
        
  //     }
  //   } else {
  //     err_counter++;
  //     flow_pressure++;
  //   }
  //   if(loops_counter >= GOOD_COUNT){
  //     set_new_flow_pressure(flow_pressure);
  //     is_testing = false;
  //     break;
  //   }
  //   // if (err_counter >= GOOD_COUNT){
  //   //   break;
  //   // }

  // }
  // loops_counter = 0;
  // err_counter = 0;
  // set_flow_pressure(0);
  // flow_close();
// }


// float step(int p){
//   set_flow_pressure(p);
//   flow_forward();
//   wait(FLOW_DURATION);
//   flow_backward();
//   wait(FLOW_DURATION);
//   return get_flowrate();
// }


void wait(int d){
  last_switch_millis = millis();
  // unsigned long last_output = millis();
  // snprintf(info_buf, sizeof(info_buf), "wait... ");
  while(is_d_testing){
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
  while(is_testing){
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



// byte check_flow(byte pressure){
//   snprintf(result_buf, sizeof(result_buf), "checking...");
//   snprintf(info_buf, sizeof(info_buf), "");
//   flow_close();
//   float rate = step(pressure);
//   for(byte i = 0; i < FINAL_LOOPS; i++){
//     float current_rate = step(pressure);
//     flow_res[i] = current_rate;
//     char rr[20];
//     char pp[20];
//     dtostrf(current_rate, 6, 2, rr);
//     dtostrf(PRESSURE_CORRECTION*pressure, 4, 2, pp);

//     snprintf(info_buf, sizeof(info_buf), "Loop %d, P=%s R=%s", i, pp, rr);
//     if(!is_equal(current_rate, rate, FINAL_RATE_PRECISION)){
//       flow_close();
//       snprintf(result_buf, sizeof(result_buf), "Bad!");
//       return i;
//     }
//   }
//   flow_close();

//   snprintf(result_buf, sizeof(result_buf), "Good!");
//   float final_flowrate = 0;
//   for(byte i = 0; i < FINAL_LOOPS; i++){
//     final_flowrate += flow_res[i];
//   }
//   final_flowrate /= FINAL_LOOPS;
//   char rr[20];
//   char pp[20];
//   dtostrf(final_flowrate, 4, 2, rr);
//   dtostrf(PRESSURE_CORRECTION*pressure, 4, 2, pp);
//   snprintf(info_buf, sizeof(info_buf), "P=%s R=%s", pp, rr);
//   return 0;
// }

void static_test(){
  static_mode = true;
  is_testing = true;
  flow_forward();
  int static_press = keep_flow(false);
  static_mode = false;
  snprintf(result_buf, sizeof(result_buf), "Waiting mode");
  snprintf(info_buf, sizeof(info_buf), "");
  set_flow_pressure(0);
  flow_close();
}


void dynamic_test(){
  dynamic_mode = true;
  slow_i_delay = 10;
  int common_loops_counter = 0;
  byte loops_counter = 0;
  // Serial.println("dynamic_test()");
  snprintf(result_buf, sizeof(result_buf), "Dynamic mode.");
  snprintf(info_buf, sizeof(info_buf), "dynamic, AO=%d", flow_pressure);
  // flow_close();
  // flow_pressure = 30+flow_rate_full*2;
  is_d_testing = true;
  flow_forward();
  // int static_press = keep_flow();
  int begin_press = flow_pressure;
  int static_press = flow_pressure;
  while (is_d_testing){
    set_flow_pressure(static_press);
    flow_backward();
    wait(10000);
    flow_forward();
    common_loops_counter++;
    while(is_d_testing){
      wait(10000);
      get_flowrate();
      if(is_equal(current_flowrate, flow_rate_full, FINAL_RATE_PRECISION) == true){
        loops_counter++;
        break;
      } else {
        loops_counter = 0;
        // if(current_flowrate < flow_rate_full){
        //   static_press++;
        // } else {
        //   static_press--;
        // }
        static_press = keep_flow();
        snprintf(result_buf, sizeof(result_buf), "loops: %d/%d", common_loops_counter, loops_counter);
        snprintf(info_buf, sizeof(info_buf), "dynamic, AO=%d", static_press);
      }
    }
    snprintf(result_buf, sizeof(result_buf), "loops: %d/%d", common_loops_counter, loops_counter);
    snprintf(info_buf, sizeof(info_buf), "dynamic, AO=%d", static_press);
    if(loops_counter>19){
      break;
    }

  }
  // set_flow_pressure(0);
  // flow_close();
  if(is_d_testing){
    snprintf(result_buf, sizeof(result_buf), "Finished for %d loops.", common_loops_counter-loops_counter);
    char begin_pp[20];
    char final_pp[20];
    dtostrf(PRESSURE_CORRECTION*begin_press, 4, 3, begin_pp);
    dtostrf(PRESSURE_CORRECTION*static_press, 4, 3, final_pp);
    snprintf(info_buf, sizeof(info_buf), "P: %s=>%s", begin_pp, final_pp);
  }
  loops_counter = 0;
  is_d_testing = false;
  // dynamic_mode = false;
}

// byte fixing_flow(){
//   // Serial.println("fixing_flow()");
//   while(is_testing){
//     loops_counter++;

//     snprintf(result_buf, sizeof(result_buf), "loops_counter %d", loops_counter);
//     flow_forward();
//     byte begin_press = keep_flow();
//     flow_backward();
//     byte finish_press = keep_flow();
//     if (is_equal(begin_press, finish_press, PRESSURE_PRECISION)){
//       // Serial.print("Flowrate:");
//       // Serial.println(current_flowrate);
//       return finish_press;
//     }
//     // snprintf(info_buf, sizeof(info_buf), "Press %d %d", begin_press, finish_press);
//   }
// }

int keep_flow_san(bool done_when_ready){
  PID_Controller pid;
  PID_Init(&pid, 0.01f, 0.0f, 0.0f, 0.0f, 4096.0f); // Kp, Ki, Kd, min, max
  pid.setpoint = flow_rate_full; // Заданное значение
  while (1) {
    float feedback = get_flowrate(); // текущее значение (например, с датчика)
    flow_pressure = PID_Compute(&pid, feedback);

    set_flow_pressure(flow_pressure);
  }
}


int keep_flow_ni(bool done_when_ready){
  unsigned long last_wait = millis();
  int step = 1;
  int dl = 1; // delay

  byte toward = 2;

  float last_flowrate = get_flowrate();

  PID_Controller pid;
  PID_Init(&pid, 0.001f, 0.25f, 0.02f, 0.0f, 4096.0f); // Kp, Ki, Kd, min, max
  // pid.setpoint = flow_rate_full; // Заданное значение

  while(is_testing){
    pid.setpoint = flow_rate_full; // Заданное значение
    unsigned long current_millis = millis();
    if (current_millis - last_wait > 10000) {
      // snprintf(result_buf, sizeof(result_buf), "10sec");
      is_flow_ready = true;
      if(done_when_ready){
        break;
      }
    } else {
      is_flow_ready = false;
    }

    if(dynamic_mode == false){
      snprintf(result_buf, sizeof(result_buf), "is r: %d.   %d,%ds, %d/%d.", is_flow_ready, (current_millis-last_wait)/1000, ((current_millis-last_wait)%1000)/100, step, dl);
      snprintf(info_buf, sizeof(info_buf), "static, tw=%d, AO=%d", toward, flow_pressure);
    } else if (dynamic_mode && is_d_testing) {
      snprintf(info_buf, sizeof(info_buf), "dyna..., tw=%d, AO=%d", toward, flow_pressure);
    }
  



    
    set_flow_pressure(flow_pressure);
    wait_t(dl);
    update_monitor();
    read_buttons();

    
    if (is_equal(current_flowrate, flow_rate_full, RATE_PRECISION, toward)){
      toward = 0;
    } else {
      last_wait = current_millis;
      float s = abs(flow_rate_full/current_flowrate-1);
      if (s < 0.03){
        step = 1;
        dl = 5000;
      } else if (s < 0.05){
        step = 1;
        dl = 1000;
      } else if (s < 0.1){
        step = 1;
        dl = 500;
      } else if (s < 0.2){
        step = 1;
        dl = 100;
      } else if(s < 0.5){
        step = 1;
        dl = 1;
      } else {
        dl = 0;
        step = 10;
        // float feedback = get_flowrate(); // текущее значение (например, с датчика)
        // flow_pressure = PID_Compute(&pid, feedback);
        // set_flow_pressure(flow_pressure);
      }
      
      float k = flow_rate_full / current_flowrate;
      if (1){
        if(k > 1){
          toward = 2;
          flow_pressure += step;
        }else{
          toward = 1;
          flow_pressure -= step;
        }
      } else {
        
      }
      
    }
    





    if (flow_pressure > 4094 && current_millis - last_wait > 8000){
      if (current_flowrate < 5){
        snprintf(result_buf, sizeof(result_buf), "Plug column or compressor");
      } else {
        snprintf(result_buf, sizeof(result_buf), "Can't reach that rate!");
      }
      char rr[20];
      char pp[20];
      dtostrf(current_flowrate, 6, 2, rr);
      dtostrf(PRESSURE_CORRECTION*flow_pressure, 4, 2, pp);
      snprintf(info_buf, sizeof(info_buf), "P=%s R=%s", pp, rr);
      is_testing = false;
      flow_pressure = 0;
      if(done_when_ready){
        break;
      }
    }


  }
  is_flow_ready = false;
  return flow_pressure;
}



void wait_stable_flow(){
  float last_f = get_flowrate();
  while(true){
    wait(3000);
    float current_f = get_flowrate();
    if(abs(current_f/last_f-1) < 0.03){
      return;
    }
    last_f = current_f;
  }
  
}


int keep_flow(){
  return keep_flow(true);
}


int keep_flow(bool done_when_ready){
  // return keep_flow_ni(done_when_ready);
  unsigned long last_wait = millis();
  unsigned long unstable_timer = millis();
  unsigned long last_wait_k = millis();
  unsigned long last_wait_i = millis();
  bool slow_i = false;
  byte toward = 2;
  // Serial.println("keep_flow()");
  while(is_testing){
    // Serial.println("Flow");
    unsigned long current_millis = millis();
    if (current_millis - last_wait > 10000) {
      // snprintf(result_buf, sizeof(result_buf), "10sec");
      is_flow_ready = true;
      if(done_when_ready){
        break;
      }
    } else {
      is_flow_ready = false;
    }
    set_flow_pressure(flow_pressure);
    // wait_stable_flow();

    if(current_millis - last_wait > slow_i_delay){
      if (current_millis - last_wait > 1000){
        slow_i_delay = 1000;
      } else {
        slow_i_delay = (current_millis - last_wait)%1000;
      }
    }
    // wait
    // Serial.println("Wait");
    
    if(dynamic_mode == false){
      snprintf(result_buf, sizeof(result_buf), "is ready: %d.   %d,%ds", is_flow_ready, (current_millis-last_wait)/1000, ((current_millis-last_wait)%1000)/100);
      snprintf(info_buf, sizeof(info_buf), "static, tw=%d, AO=%d", toward, flow_pressure);
    } else if (dynamic_mode && is_d_testing) {
      snprintf(info_buf, sizeof(info_buf), "dyna..., tw=%d, AO=%d", toward, flow_pressure);
    }
    
    // float x = get_flowrate();
    update_monitor();
    read_buttons();

      
    

    // Serial.println("End wait");
    // end wait
    // if(abs(flow_rate_full/current_flowrate-1)>0.05 || abs(flow_rate_full-current_flowrate>5)){
    //   slow_i = false;
    // }
    if(abs(flow_rate_full-current_flowrate>slow_i_range)){
      slow_i = false;
    }
    if (is_equal(current_flowrate, flow_rate_full, RATE_PRECISION, toward)){
      toward = 0;
      slow_i = true;
    } else {
      // last_wait = current_millis;
      float k = flow_rate_full / current_flowrate;
      if (k >= 1){
        toward = 2;
      } else {
        toward = 1;
      }
      if ((abs(k-1) > 0.2) && flow_pressure < 4090 && current_millis - last_wait_k > 3000){
        last_wait_k = millis();
        if (0.9 * k * flow_pressure > 4095.0){
          flow_pressure = 4095;
        } else {
          flow_pressure *= 0.9*k;
        }
        // snprintf(info_buf, sizeof(info_buf), "pressure H %d", flow_pressure);
      } else {
        if(slow_i && current_millis - last_wait_i < slow_i_delay){
          continue;
        }
        last_wait_i = current_millis;
        if (current_flowrate > flow_rate_full){
          if (flow_pressure > 0){
            if (current_flowrate - flow_rate_full > 10){
              flow_pressure -= 10;
            } else {
              flow_pressure--;
            }
          }
        } else {
          if (flow_pressure < 4095){
            if(flow_rate_full - current_flowrate > 10){
              flow_pressure += 10;
            } else {
              flow_pressure++;
            }
          }
        }
        // snprintf(info_buf, sizeof(info_buf), "pressure L %d", flow_pressure);
      }
    }
    if(is_equal(current_flowrate, flow_rate_full, FINAL_RATE_PRECISION)){
      unstable_timer = millis();
    } else {
      last_wait = current_millis;
      if(current_millis-unstable_timer>SLOW_I_DELAY_BEFORE_CHANGE){
        if(slow_i){
          slow_i_range--;
        } else {
          slow_i_range++;
        }
        if(slow_i_range > MAX_SLOW_I_RANGE){
          slow_i_range = MAX_SLOW_I_RANGE;
        }
        if(slow_i_range < MIN_SLOW_I_RANGE){
          slow_i_range = MIN_SLOW_I_RANGE;
        }
      }
    }
    // char buffer[20];
    // snprintf(buffer, sizeof(buffer), "Counter %%", counter);
    // const char* cstr = buffer;
    // update_monitor(buffer);
    // Serial.print("Flowrate: ");
    // Serial.println(current_flowrate);
    // Serial.println(flow_pressure);
    // Serial.print("Counter: "); Serial.println(counter);
    
    if (flow_pressure > 4094 && current_millis - last_wait > 10000){
      if (current_flowrate < 5){
        snprintf(result_buf, sizeof(result_buf), "Plug column or compressor");
      } else {
        snprintf(result_buf, sizeof(result_buf), "Can't reach that rate!");
      }
      char rr[20];
      char pp[20];
      dtostrf(current_flowrate, 6, 2, rr);
      dtostrf(PRESSURE_CORRECTION*flow_pressure, 4, 2, pp);
      snprintf(info_buf, sizeof(info_buf), "P=%s R=%s", pp, rr);
      is_testing = false;
      flow_pressure = 0;
      if(done_when_ready){
        break;
      }
    }
  }
  is_flow_ready = false;
  return flow_pressure;
}












void setup() {
  // put your setup code here, to run once:
  // Serial.begin(9600);
  // Serial.setTimeout(1);
  // Serial.print("Starting...      ");

  pinMode(FLOWRATE_PIN, INPUT_ANALOG);
  analogReference(AR_DEFAULT);  
  analogReadResolution(12);
  analogWriteResolution(12);
  // Включаем тактирование порта B
  // RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

  // // Настройка PB5 как вход без pull-up/pull-down
  // GPIOB->CRL &= ~(0xF << (5 * 4));  // Очистить биты настройки
  // GPIOB->CRL |= (0x00 << (5 * 4));  // Input floating

  EEPROM.begin();
  int data = 0;
  data = EEPROM.read(0);
  flow_rate_full = EEPROM.read(1);
  flow_rate_single = EEPROM.read(2);
  if(data >= 0 && data <= 255){
    flow_pressure = data;
  }

  // analogReference(INTERNAL);

  pinMode(VALVE_1_PIN, OUTPUT);
  pinMode(VALVE_2_PIN, OUTPUT);
  // pinMode(FLOWRATE_PIN, INPUT);
  pinMode(GEAR_PIN, OUTPUT);
  // pinMode(pressure_pin, OUTPUT);
  pinMode(RED_LED_PIN, OUTPUT);
  pinMode(GREEN_LED_PIN, OUTPUT);
  pinMode(BTN_START_PIN, INPUT);
  pinMode(BTN_UP_PIN, INPUT);
  pinMode(BTN_DOWN_PIN, INPUT);
  pinMode(BTN_MODE_PIN, INPUT);

  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  
  // char buffer[12] = "Starting...";
//  itoa(x, buffer, 10);
  oled.clear();
  oled.drawRow(1, "Starting...");
  oled.drawRow(2, "Version 2.7");
  oled.update();


  snprintf(result_buf, sizeof(result_buf), "Done!");
  snprintf(info_buf, sizeof(info_buf), "");
  


  // digitalWrite(GREEN_LED_PIN, HIGH);
  // delay(200);
  // digitalWrite(RED_LED_PIN, HIGH);
  // delay(500);
  digitalWrite(VALVE_1_PIN, HIGH);
  delay(1000);
  digitalWrite(VALVE_1_PIN, LOW);
  delay(500);
  digitalWrite(VALVE_2_PIN, HIGH);
  delay(1000);
  digitalWrite(VALVE_2_PIN, LOW);
  delay(500);
  digitalWrite(GREEN_LED_PIN, LOW);
  digitalWrite(RED_LED_PIN, LOW);


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
  // put your main code here, to run repeatedly:
  
  // that's for debugging
  // if (Serial.available()){ 
  //   int ss = Serial.readString().toInt();
  //   if(ss > 255){
  //     if(ss == 300){
  //       digitalWrite(VALVE_1_PIN, LOW);
  //     } else if (ss == 301){
  //       digitalWrite(VALVE_1_PIN, HIGH);
  //     }else if (ss == 400){
  //       digitalWrite(VALVE_2_PIN, LOW);
  //     }else if (ss == 401){
  //       digitalWrite(VALVE_2_PIN, HIGH);
  //     } else if (ss == 500){
  //       digitalWrite(RED_LED_PIN, LOW);
  //     }else if (ss == 501){
  //       digitalWrite(RED_LED_PIN, HIGH);
  //     }else if (ss == 600){
  //       digitalWrite(GREEN_LED_PIN, LOW);
  //     }else if (ss == 601){
  //       digitalWrite(GREEN_LED_PIN, HIGH);
  //     }else if (ss == 700){
  //       logs = !logs;
  //     }else if (ss == 1000){
  //       // leds_state = testing();
  //     }else{
  //       // Serial.println(is_equal(ss, 1000, 5));
  //     }
  //   } else {
  //     analogWrite(GEAR_PIN, ss);
  //     if (ss == 0){
  //       u8g2.begin();
  //     }
    // }
    // lcd_write(ss);
  // }

  // bool btn_state = !digitalRead(BTN_START_PIN);
  // // Serial.println(btn_state);
  // if(btn_state==true){
  //   // leds_state = testing();
  //   // dynamic_mode = true;
  //   static_test();
    
    
  //   // Serial.print("----");
  // }

  // current_flowrate = get_flowrate();

  if(logs){
    // int pressure = analogRead(pressure_pin);
  float rate = 0.005 * RATE_VOLTAGE_CORRECTION * analogRead(FLOWRATE_PIN);
  // Serial.print("Flowrate: ");
  // Serial.print(rate);
  // Serial.print("v      (");
  // Serial.print(PFMV505_flow(rate, RATE_VOLTAGE_OFFSET));
  // Serial.print("sm3/m)");
  // Serial.println();
  }
  if(leds_state == 0){ 
    enable(RED_LED_PIN, false);
    enable(GREEN_LED_PIN, false);
  } else if (leds_state == 1){
    enable(RED_LED_PIN, false);
    enable(GREEN_LED_PIN, true);
  } else {
    enable(RED_LED_PIN, true);
    blink_led(GREEN_LED_PIN, leds_state - 1);
  }
  update_monitor();
  read_buttons();
  // update_monitor();
}
