
#define op 11
#define dl 2

unsigned int d;

void setup() {
  // put your setup code here, to run once:
  d = 1000;
  pinMode(op, OUTPUT);
  pinMode(op, LOW);
  
}


void sb(unsigned int n){
  for(unsigned int i = 0; i < n; i++){
    delay(dl);
    digitalWrite(op, HIGH);
    delay(dl*2);
    digitalWrite(op, LOW);
    delay(dl);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
//  digitalWrite(8, HIGH);
//  delay(10);
//  digitalWrite(8, LOW);
//  delay(10);
  if (Serial.available()){ 
    d = Serial.readString().toInt();
    Serial.print(d);
    sb(d);
    Serial.println(".");
  }
 

}
