void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long int m = millis();
  Serial.println(m);
  for(unsigned long int i = 0; i <= 100000; i++){
    int d =1;
  }

}
