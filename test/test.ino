#include <Arduino.h>
#include <U8g2lib.h>


#include <OLEDHelper.h>
#include "Button.h"



// Создание объекта дисплея (4-проводный Software SPI)
U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(
  /*  */ U8G2_R2,
  /* clock=*/ 13,      // CLK / SCK
  /* data=*/  11,      // MOSI / DATA
  /* cs=*/    10,      // CS   (Chip Select)
  /* dc=*/    17,       // DC   (Data/Command)
  /* reset=*/ 16        // RST  (Reset)
);

OLEDHelper oled(u8g2);

// Button btn_up(3);
// Button btn_down(2);
Button btn_mode(15);

// Примеры переменных
int var1 = 123;
float var2 = 4.56;
bool var3 = true;

void setup(void) {
  u8g2.begin(); // Инициализация дисплея
  u8g2.setFont(u8g2_font_ncenB08_tr);

  oled.clear();
  oled.drawRow(1, "Starting...");
  oled.update();

  // btn_up.begin();
  // btn_down.begin();
  btn_mode.begin();


  Serial.begin(115200);
  Serial.println("Done!");
  delay(800);
}

bool md_full = false;

void read_buttons(){

  // btn_up.update();
  // btn_down.update();
  btn_mode.update();
  // if (btn_up.isPressed()){
  //   var1 += 1;
  //   Serial.println("UP pressed!");
  // }
  // if (btn_down.isPressed()){
  //   var1 -= 1;
  //   Serial.println("DOWN pressed!");
  // }
  if (btn_mode.isPressed()){
    Serial.println("MODE pressed!");
    md_full = !md_full;
    if(md_full){
      oled.drawMode("Full", 80);
    } else {
      oled.drawMode("Single", 120);
    }
    oled.update();
  }
}

void loop(void) {
  oled.clear();

  // read_buttons();

  // oled.drawVariable(2, 10, "Var1:  ", var1);
  // oled.drawVariable(2, 22, "Var2:  ", var2, 2);
  // oled.drawVariable(2, 34, "Var3:  ", var3);

  oled.drawMode("Full", 123);
  oled.drawRate(var1);
  oled.drawPressure(var2);
  // oled.drawResult("Good!");
  oled.drawRow(4, "Good!");
  oled.drawResultInfo(var1, var2);

  oled.update();

  // // Нарисовать текст
  // u8g2.setFont(u8g2_font_ncenB08_tr); // Выбрать шрифт

  // // u8g2.drawStr(1, 10, ("test" + String(var1)).c_str());
  // u8g2.drawStr(2, 10, ("Var1:   " + String(var1)).c_str());
  // u8g2.drawStr(2, 22, ("Var2:   " + String(var2, 2)).c_str());
  // u8g2.drawStr(2, 34, ("Var3:   " + String(var3 ? "ON" : "OFF")).c_str());
  // u8g2.drawStr(2, 46, ("Var4:   None"));
  // u8g2.drawStr(2, 58, ("Var5:   None"));
  // u8g2.sendBuffer(); // Отправить буфер на дисплей

  delay(500); // Обновление раз в полсекунды
}