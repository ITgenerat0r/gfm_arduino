#include "Button.h"

Button::Button(int pin) {
  _pin = pin;
}

void Button::begin() {
  pinMode(_pin, INPUT_PULLUP);
  _lastState = digitalRead(_pin);
  _currentState = _lastState;
}

void Button::update() {
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

bool Button::isPressed() {
  return _pressedFlag;
}

bool Button::isLongPressed() {
  return _longPressedFlag;
}