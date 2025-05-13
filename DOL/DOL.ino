// DM566 config

// Full current
// SW1 - off
// SW2 - off
// SW3 - off
// SW4 - on

// 400 steps for period
// SW5 - off
// SW6 - on
// SW7 - on
// SW8 - on






#define pul_pin 20
#define dir_pin 19
#define ena_pin 18

#define MIN_ICED 135
#define MAX_ICED 1300


bool dir = false;


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.setTimeout(1);

  
  pinMode(pul_pin, OUTPUT);
  pinMode(dir_pin, OUTPUT);
  pinMode(ena_pin, OUTPUT);

  digitalWrite(ena_pin, LOW);
  delay(1);
  digitalWrite(dir_pin, LOW);
  delay(1);
  digitalWrite(pul_pin, HIGH);
  delay(1);

}


void spin_old(int n){
  float d = 1;
  for(int i = 0; i < n; i++){
    digitalWrite(pul_pin, LOW);
//    delay(d);
    digitalWrite(pul_pin, HIGH);
    delay(d);
  }
}

// n - number of steps, dly - delay in ms/10 for semistep, it's means 20dly == 2ms for one step
void spin(int n, int dly){
  int pass = 0;
  n *= 2;
  bool state = true;
//  unsigned long int last_ms = millis();
  while(n > 0){
//    unsigned long int current_ms = millis();
//    last_ms = current_ms();
//    if (current_ms > 0){
//      pass = pass - (pass % 10) + 10;
//    } 
//    pass++;
    if(++pass > dly){
//      Serial.println(pass);
      pass = 0;
      if(state){
        digitalWrite(pul_pin, LOW);
      } else {
        digitalWrite(pul_pin, HIGH);
      }
      state = !state;
      n--;
    }
//    Serial.println(pass);
    
    
  }
}


void spin2(int n, int dly){
  int pass = 0;
//  unsigned long int last_ms = millis();
  while(n > 0){
//    unsigned long int current_ms = millis();
//    last_ms = millis();
    if (++pass > dly){
//      pass = pass - (pass % 10) + 10;
//      Serial.println(pass);
      pass = 0;
    } 
//    else {
//      pass++;
//      if (pass > dly){
//        pass = 0;
//      }
//    }
//    pass++;
//    last_ms = current_ms;
    if(pass == 0){
      digitalWrite(pul_pin, LOW);
      digitalWrite(pul_pin, HIGH);
      n--;
    }
  }
}


void runner(int min_speed, int max_speed, void (*spin)(int, int)){
  for(int i = min_speed; i >= max_speed; i--){
    spin(50, i);
  }
  for(int i = max_speed; i <= min_speed; i++){
    spin(50, i);
  }
}


// ice table
int table_iced[] =  {1200, 1000, 850, 700, 600, 500, 400, 300, 350, 200, 150, 125, 100};
int table_steps[] = {   3,   5,  10,  15,  20,  35,  50,  70,  100, 200, 300, 400, 500};


void fly(bool direction, int32_t duration, void (*spin)(int, int)){
  int32_t log = -1;
  if(direction){
    //    dir = false;
    digitalWrite(dir_pin, HIGH);
    delay(1);
  } else {
    //    dir = true;
    digitalWrite(dir_pin, LOW);
    delay(1);
  }

  int32_t current_duration = duration / 2;
  // duration -= current_duration;

  byte pos = 0;
  int steps = table_steps[pos];
  int iced = table_iced[pos];

  // speed up
  bool key_done = false;
  int min_i = MIN_ICED;
  int32_t step_counter = 0;
  for(int i = MAX_ICED; i >= MIN_ICED; i--){
    current_duration -= steps;
    if(current_duration<0){
      steps += current_duration;
      current_duration = 0;
      key_done = true;
    }
    spin(steps, i);
    step_counter += steps;
    if(key_done){
      min_i = i;
      break;
    }
    if(i < table_iced[pos] && sizeof(table_iced)-1 > pos){
      steps = table_steps[++pos];
    }
  }
  // max speed
  duration -= step_counter * 2;
  log = duration;
  while(duration > 0){
    if(duration<1000){
      spin(duration, min_i);
      break;
    } else {
      spin(1000, min_i);
      duration -= 1000;
    }
    
  }
  duration = step_counter;

  



  // slow down
  key_done = false;
  int32_t down_counter = 0;
  for(int i = min_i; i <= MAX_ICED; i++){
    duration -= steps;
    if(duration < 0){
      steps += duration;
      duration = 0;
      key_done = true;
    }
    spin(steps, i);
    down_counter += steps;
    if(key_done){
      break;
    }
    if(i > table_iced[pos] && pos > 0){
      steps = table_steps[--pos];
    }
  }
  spin(duration, table_iced[0]);
  down_counter += duration;


  Serial.print("Speed up takes ");
  Serial.print(step_counter);
  Serial.println(" steps.");

  Serial.print("High speed takes ");
  Serial.print(log);
  Serial.println(" steps.");

  Serial.print("Slow down takes ");
  Serial.print(down_counter);
  Serial.println(" steps.");

  Serial.print("Log: (duration)"); Serial.println(duration);


}




int32_t ss = 100;
void loop() {
  // put your main code here, to run repeatedly:

  if (Serial.available()){ 
    Serial.println(ss);
      // ss = Serial.readString().toInt();
    int32_t st = Serial.parseInt();
    if (st > 0){
      ss = 100 * st;
      
      // ss = 100 * Serial.parseInt();
        // uint32_t xx = 0-1;
      Serial.print("--------------------- \nSet steps: "); Serial.println(ss);
        
    // runner(1200, ss, spin); 
      fly(dir, ss, spin);
      
      Serial.println("-----------------");
    } else {
      dir = !dir;
      Serial.print("dir: ");
      Serial.println(dir);
    }
  }


}
