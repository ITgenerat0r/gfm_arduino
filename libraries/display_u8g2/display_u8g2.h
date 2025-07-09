#ifndef DISPLAY_U8G2_H
#define DISPLAY_U8G2_H

#include <Arduino.h>
#include <U8g2lib.h>


U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(
  /*  */ U8G2_R2,
  /* clock=13*/ 13,      // CLK / SCK
  /* data=11*/  11,      // MOSI / DATA
  /* cs=10*/    10,      // CS   (Chip Select)
  /* dc=9(17)*/    17,       // DC   (Data/Command)
  /* reset=8(16)*/ 16        // RST  (Reset)
);



void begin();
void update();


#endif