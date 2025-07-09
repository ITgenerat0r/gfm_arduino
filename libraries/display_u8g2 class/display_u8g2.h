#ifndef DISPLAY_U8G2_H
#define DISPLAY_U8G2_H

#include <Arduino.h>
#include <U8g2lib.h>

class Display_u8g2 {
  public:
    Display_u8g2();

    void begin();
    void update(); // Вызывается каждый loop() для обновления состояния

    bool isPressed();     // Короткое нажатие
    bool isLongPressed(); // Долгое нажатие

  private:
    int _pin;

    unsigned long _pressStartTime = 0;
    const long _longPressThreshold = 1000; // Порог долгого нажатия в мс

    int _lastState = HIGH;
    int _currentState;

    bool _pressedFlag = false;
    bool _longPressedFlag = false;

    unsigned long _lastDebounceTime = 0;
    const long _debounceDelay = 50; // Задержка дебаунса
};

#endif