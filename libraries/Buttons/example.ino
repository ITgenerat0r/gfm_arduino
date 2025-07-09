// MyProject.ino

#include "Arduino.h"
#include "Button.h"

Button myButton(A0); // Подключаем к аналоговому пину A0

void setup() {
  Serial.begin(9600);
  myButton.begin();
}

void loop() {
  myButton.update();

  if (myButton.isPressed()) {
    Serial.println("Кнопка коротко нажата");
  }

  if (myButton.isLongPressed()) {
    Serial.println("Кнопка долго нажата");
  }

  delay(10); // Можно сделать меньше, так как update сам управляет временем
}