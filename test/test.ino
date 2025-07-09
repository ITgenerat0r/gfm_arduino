#include <Arduino.h>
#include <U8g2lib.h>

// Создание объекта дисплея (4-проводный Software SPI)
U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(
  /*  */ U8G2_R2,
  /* clock=*/ 13,      // CLK / SCK
  /* data=*/  11,      // MOSI / DATA
  /* cs=*/    10,      // CS   (Chip Select)
  /* dc=*/    17,       // DC   (Data/Command)
  /* reset=*/ 16        // RST  (Reset)
);

// Примеры переменных
int var1 = 123;
float var2 = 4.56;
bool var3 = true;

void setup(void) {
  u8g2.begin(); // Инициализация дисплея
  Serial.begin(115200);
  Serial.println("Done!");
}

void loop(void) {
  char t[10] = String("test").c_str();
  Serial.println(("test" + String(var1)).c_str());
  u8g2.clearBuffer(); // Очистить буфер

  // Нарисовать текст
  u8g2.setFont(u8g2_font_ncenB08_tr); // Выбрать шрифт

  // u8g2.drawStr(1, 10, ("test" + String(var1)).c_str());
  u8g2.drawStr(2, 10, ("Var1:   " + String(var1)).c_str());
  u8g2.drawStr(2, 22, ("Var2:   " + String(var2, 2)).c_str());
  u8g2.drawStr(2, 34, ("Var3:   " + String(var3 ? "ON" : "OFF")).c_str());
  u8g2.drawStr(2, 46, ("Var4:   None"));
  u8g2.drawStr(2, 58, ("Var5:   None"));
  u8g2.sendBuffer(); // Отправить буфер на дисплей

  delay(500); // Обновление раз в полсекунды
}