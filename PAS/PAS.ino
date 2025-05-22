



#define clk 8
#define mosi 9
#define ss 12
#define u_pin 10
#define imp_pin 11

#define async_delay 1

//int ss_ar[5] = {12};
unsigned long int x;



void setup() {
  x = 21846;
  Serial.begin(115200);
  Serial.setTimeout(1);
  
  pinMode(clk, OUTPUT);
  pinMode(mosi, OUTPUT);
  pinMode(ss, OUTPUT);
//  for(int i = 0; i<5; i++){
//    pinMode(ss_ar[i], OUTPUT);
//  }
  pinMode(u_pin, OUTPUT);
  pinMode(imp_pin, OUTPUT);


  digitalWrite(clk, LOW);
  digitalWrite(mosi, LOW);
  digitalWrite(ss, HIGH);
//  for(int i = 0; i<5; i++){
//    digitalWrite(ss_ar[i], HIGH);
//  }
  analogWrite(imp_pin, 0);
  analogWrite(u_pin, 255);
  

  
//  Serial.write("The program has begun.");
}


void clocks(byte n){
  for(byte i = 0; i < n; i++){
    delay(async_delay);
    digitalWrite(clk, HIGH);
    delay(async_delay*2);
    digitalWrite(clk, LOW);
    delay(async_delay);
  }
}


void send_bit(bool b){
  if(b){
    digitalWrite(mosi, HIGH);
  }else{
    digitalWrite(mosi, LOW);
  }
//  delay(async_delay);
  
  clocks(16);
}


void transfer_async(unsigned int data){
//  Serial.print("data: ");
//  Serial.println(data, HEX);
  digitalWrite(mosi, HIGH);
  digitalWrite(clk, LOW);
  delay(async_delay);

  clocks(8);
  send_bit(false);
  
  for(byte i = 15; i >= 0; i--){
    bool b = (data>>i)&1;
    send_bit(b);
    if (i == 0) break;
  }
  send_bit(true);
  digitalWrite(mosi, LOW);
  clocks(8);
  delay(async_delay);
  
  digitalWrite(mosi, HIGH);
  digitalWrite(clk, HIGH);
}



void transfer_3wire(unsigned int data){
//  Serial.print("data: ");
//  Serial.println(data, HEX);
    delay(1);
  for(byte i = 15; i >= 0; i--){
    bool b = (data>>i)&1;
    digitalWrite(clk, LOW);
    delay(1);
    if(b){
      digitalWrite(mosi, HIGH);
    }else{
      digitalWrite(mosi, LOW);
    }
    delay(1);
    digitalWrite(clk, HIGH);
    delay(1);
    if (i == 0) break;
  }
  digitalWrite(ss, LOW);
  digitalWrite(ss, HIGH);
//  for(int i = 0; i<5; i++){
//    digitalWrite(ss_ar[i], LOW);
//  }
//  for(int i = 0; i<5; i++){
//    digitalWrite(ss_ar[i], HIGH);
//  }
  digitalWrite(mosi, LOW);
  digitalWrite(clk, LOW);
}



void loop() {
  if (Serial.available()){ 
    x = Serial.readString().toInt();
    if(x < 0){
      Serial.print("PAS");
    } else {
      Serial.print(x, HEX);
    }
    
//    Serial.print(", ");
    unsigned int i_value = x&0xffff;
    byte u_value = (x>>16)&0xff;
    u_value ^= 0xff;
    byte imp_value = (x>>24)&0xff;
//    if(u_value < 0x2D){u_value = 0x2D;}
//    Serial.print("(");
//    Serial.print(imp_value, HEX);
//    Serial.print(", ");
//    Serial.print(i_value, HEX);
//    Serial.print(", ");
//    Serial.print(u_value, HEX);
//    Serial.print(")");
    transfer_3wire(i_value);
    analogWrite(u_pin, u_value);
    analogWrite(imp_pin, imp_value);
//    Serial.println(".");
  }
  
  
//  transfer_async(x);
delay(10);
}
