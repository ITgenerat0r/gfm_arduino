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
#define ss 10
#define dc 17
#define res 16


// micro lattepanda
// #define clk 3
// #define mosi 2
// #define ss 4
// #define dc 18
// #define res 9

// U8G2_SSD1309_128X64_NONAME0_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);    

// U8G2_SSD1309_128X64_NONAME2_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);  
// U8G2_SSD1309_128X64_NONAME2_2_4W_SW_SPI u8g2(U8G2_R3, /* clock=*/ 3, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 16, /* reset=*/ 9);
U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(U8G2_R2, /* clock=*/ clk, /* data=*/ mosi, /* cs=*/ ss, /* dc=*/ dc, /* reset=*/ res);

void setup(void) {
	Serial.begin(115200);

  /* U8g2 Project: SSD1306 Test Board */
  //pinMode(10, OUTPUT);
  //pinMode(9, OUTPUT);
  //digitalWrite(10, 0);
  //digitalWrite(9, 0);		

  /* U8g2 Project: T6963 Test Board */
  //pinMode(18, OUTPUT);
  //digitalWrite(18, 1);	

  /* U8g2 Project: KS0108 Test Board */
  //pinMode(16, OUTPUT);
  //digitalWrite(16, 0);	

  /* U8g2 Project: LC7981 Test Board, connect RW to GND */
  //pinMode(17, OUTPUT);
  //digitalWrite(17, 0);	

  /* U8g2 Project: Pax Instruments Shield: Enable Backlight */
  //pinMode(6, OUTPUT);
  //digitalWrite(6, 0);	

  u8g2.begin();  
  // u8g2.firstPage();
}

int counter = 0;
int x = 123;

void loop(void) {
	
	if (Serial.available()){ 
    	x = Serial.readString().toInt();
	    Serial.println(x);
	    if (x == 0){
	    	u8g2.begin(); 
	    }
	}
	// // u8g2.firstPage();
	// u8g2.drawVLine(5,0,10);
 // //    u8g2.drawHLine(0,31,10);   

	// // u8g2.setFont(u8g2_font_helvB08_tr);
	// u8g2.setFont(u8g2_font_u8glib_4_tr);
	// // u8g2.drawStr(0, 20, "Hello World! 123");
	// u8g2.drawStr(0, 40, counter);

	// u8g2.updateDisplay();
	// u8g2.refreshDisplay();

	char buffer[4];
	itoa(x, buffer, 10);
	u8g2.clearBuffer();
	// u8g2.setDisplayRotation(U8G2_R2);
	// u8g2.setFont(u8g2_font_ncenB08_tr);
	u8g2.setFont(u8g2_font_fub42_tr);
	u8g2.drawStr(5, 50, buffer);
	u8g2.sendBuffer();









  // u8g2.firstPage();
  // do {
  //   // u8g2.drawHLine(0,0,10);
  //   // u8g2.drawHLine(0,31,10);    

  //   u8g2.setFont(u8g2_font_helvB08_tr);
  //   // u8g2.setFont(u8g2_font_micro_mn);
  //   u8g2.drawStr(0,20,"Hello World! 123");
  // } while ( u8g2.nextPage() );
  delay(1000);
}