#include "Display_u8g2.h"

Display_u8g2::Display_u8g2() {
  U8G2_SSD1309_128X64_NONAME2_F_4W_SW_SPI u8g2(
    /*  */ U8G2_R2,
    /* clock=13*/ 13,      // CLK / SCK
    /* data=11*/  11,      // MOSI / DATA
    /* cs=10*/    10,      // CS   (Chip Select)
    /* dc=9(17)*/    17,       // DC   (Data/Command)
    /* reset=8(16)*/ 16        // RST  (Reset)
  );
}

void Display_u8g2::begin() {
  u8g2.begin();
}

void Display_u8g2::update() {
  int reading = digitalRead(_pin);

  if (reading != _lastState) {
    _lastDebounceTime = millis();
  }

  if ((millis() - _lastDebounceTime) > _debounceDelay) {
    if (reading != _currentState) {
      _currentState = reading;

      if (_currentState == LOW) {
        // Кнопка нажата
        _pressStartTime = millis();
        _pressedFlag = false;
        _longPressedFlag = false;
      } else {
        // Кнопка отпущена
        if (millis() - _pressStartTime < _longPressThreshold && !_pressedFlag) {
          _pressedFlag = true;
        }
      }
    }
  }

  if (_currentState == LOW && !(_pressedFlag || _longPressedFlag)) {
    if (millis() - _pressStartTime >= _longPressThreshold) {
      _longPressedFlag = true;
    }
  }

  _lastState = reading;
}

bool Display_u8g2::isPressed() {
  return _pressedFlag;
}

bool Display_u8g2::isLongPressed() {
  return _longPressedFlag;
}