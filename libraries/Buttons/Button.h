#ifndef BUTTON_H
#define BUTTON_H

#include <Arduino.h>

class Button {
  public:
    Button(int pin);

    void begin();
    void update(); // Вызывается каждый loop() для обновления состояния

    bool isPressed();     // Короткое нажатие
    bool isLongPressed(); // Долгое нажатие

  private:
    int _pin;

    unsigned long _pressStartTime = 0;
    const long _longPressThreshold = 500; // Порог долгого нажатия в мс

    int _lastState = HIGH;
    int _currentState;

    bool _pressedFlag = false;
    bool _longPressedFlag = false;

    unsigned long _lastDebounceTime = 0;
    const long _debounceDelay = 50; // Задержка дебаунса
};

#endif