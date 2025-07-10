// OLEDHelper.h

#ifndef OLEDHELPER_H
#define OLEDHELPER_H

#include <Arduino.h>
#include <U8g2lib.h>

class OLEDHelper {
  public:
    OLEDHelper(U8G2& display); // Конструктор

    void clear();                      // Очистить экран
    void update();                     // Обновить дисплей (отправить буфер)
    
    void drawVariable(int x, int y, const char* label, int value);
    void drawVariable(int x, int y, const char* label, float value, int decimalPlaces = 2);
    void drawVariable(int x, int y, const char* label, bool value);

    void drawMode(const char* md, int val);
    void drawRate(const float val);
    void drawPressure(const float val);
    void drawResult(const char* res);
    void drawResultInfo(float rate, float pressure);
    void drawRow(byte pos, const char* value);

  private:
    U8G2* _display;
    static const uint8_t _bufferSize = 20;
    char _buffer[_bufferSize];

    void drawLabelValue(int x, int y, const char* label, const char* value);
};

#endif