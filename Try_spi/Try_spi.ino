#include <SPI.h>



#define u_pin 7
#define t_pin 5
#define ss 4

int x;
unsigned long spi_speed = 14000000;
SPISettings spi_sets(spi_speed, MSBFIRST, SPI_MODE0);


void setup() {
  x = 0;
  Serial.begin(115200);
  Serial.setTimeout(1);
  pinMode(u_pin, OUTPUT);
  pinMode(t_pin, OUTPUT);
  pinMode(ss, OUTPUT);


//  SPI.beginTransaction(spi_sets);
  SPI.begin();
  
//  SPI.begin();
//  SPI.setClockDivider(SPI_CLOCK_DIV8);
  digitalWrite(ss, HIGH);
//  Serial.write("The program has begun.");
}

void loop() {
  if (Serial.available()){ 
    x = Serial.readString().toInt();
    Serial.print(x+1);
    if(x >= 0){
      byte a = 0, b = 0;
      b = byte(x); // 0x2f
      Serial.print("send(");
      Serial.print(b);
      Serial.print("); ");
      unsigned long tb = millis();
      digitalWrite(ss, LOW);
      SPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));
      a = SPI.transfer(x&0xff);
      SPI.endTransaction();
      digitalWrite(ss, HIGH);
      unsigned long te = millis();
      Serial.print("received: (");
      Serial.print(a);
      Serial.print("); ");
      
      Serial.print(" executed for: ");
      long delt = te - tb;
      Serial.print(te-tb);
      Serial.print("ms.   ");
      
    }
    Serial.println(".");
  }
  if (x < 0){x = 0;}
  if (x > 255){x = 255;}
//  analogWrite(u_pin, x);
//  analogWrite(t_pin, x);
  
  delay(10);
}
