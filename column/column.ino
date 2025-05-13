
// digital
#define valve_1_pin 2
#define valve_2_pin 3
#define green_led_pin 5
#define gear_pin 6
#define red_led_pin 8
#define button_pin 9

// analog
#define flowrate_pin 0
// #define pressure_pin 1


#include <Arduino.h>
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif


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
#define FLOW_PRESSURE 60

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


float flow(const bool toward){
  if(toward){
    flow_forward();
  }else{
    flow_backward();
  }
  set_flow_pressure(FLOW_PRESSURE);
  delay(100);

  int precise = 1; // in percent
  float flowrate = 0;
  int n = 4000;
  int stable = 30;
  while(n--){
    float current_flowrate = get_flowrate();
    Serial.print(current_flowrate);
    Serial.print("sm3/m)");
    Serial.println();
    lcd_write(current_flowrate);
    if (is_equal(current_flowrate, flowrate, precise)){
      if(!--stable){
        Serial.print("stable when n = ");
        Serial.println(n);
        n = 0;
      }
    }
    flowrate = current_flowrate;
    delay(100);
  }
  set_flow_pressure(0);
  flow_close();
  Serial.println("The flowrate has been established!");
  return flowrate;
}


byte testing(){
  Serial.println("Testing is started...");
  flow_close();
  enable(red_led_pin, false);
  enable(green_led_pin, false);
  byte result = 0;
  bool toward = false;
  float last_flowrate = 0;
  int stable = 3;
  byte precise = 1; // in percent
  float current_flowrate = 0;
  int n = 100;
  while(--n > 0){
    current_flowrate = flow(toward);
    if(is_equal(current_flowrate, last_flowrate, precise)){
      Serial.println("Equal");
      if(--stable < 0){
        Serial.print("n = ");
        Serial.println(n);
        Serial.println("Testing done!");
        n=0;
      }
    }else{
      Serial.println("Not equal");
    }
    toward = !toward;
    last_flowrate = current_flowrate;
  }
  if (current_flowrate > pre_max_rate){
    result = 4;
  } else if (current_flowrate < pre_min_rate && current_flowrate > max_rate){
    result = 3;
  } else if (current_flowrate < min_rate){
    result = 2;
  } else {
    result = 1;
  }
  return result;
}


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
  u8g2.drawStr(5, 50, buffer);
  u8g2.sendBuffer();
}



void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setTimeout(1);
  Serial.print("Starting...      ");

  analogReference(INTERNAL);

  pinMode(valve_1_pin, OUTPUT);
  pinMode(valve_2_pin, OUTPUT);
  pinMode(flowrate_pin, OUTPUT);
  pinMode(gear_pin, OUTPUT);
  // pinMode(pressure_pin, OUTPUT);
  pinMode(red_led_pin, OUTPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(button_pin, INPUT);




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

  u8g2.begin();

  Serial.println("Done!");
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
        leds_state = testing();
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
    leds_state = testing();
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
}
