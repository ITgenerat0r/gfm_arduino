// OLEDHelper.cpp

#include "OLEDHelper.h"
#include <stdio.h>
#include <string.h>

OLEDHelper::OLEDHelper(U8G2& display) {
  _display = &display;
}

void OLEDHelper::clear() {
  _display->clearBuffer();
}

void OLEDHelper::update() {
  _display->sendBuffer();
}

// void OLEDHelper::drawLabelValue(int x, int y, const char* label, const char* value) {
//   snprintf(_buffer, BUFFER_SIZE, "%s %s", label, value);
//   _display->drawStr(x, y, _buffer);
// }

// void OLEDHelper::drawVariable(int x, int y, const char* label, int value) {
//   snprintf(_buffer, BUFFER_SIZE, "%d", value);
//   // itoa(decimalPlaces, _buffer, BUFFER_SIZE);
//   drawLabelValue(x, y, label, _buffer);
// }

// void OLEDHelper::drawVariable(int x, int y, const char* label, float value, int decimalPlaces) {
//   dtostrf(value, 1, decimalPlaces, _buffer);
//   drawLabelValue(x, y, label, _buffer);
// }

// void OLEDHelper::drawVariable(int x, int y, const char* label, bool value) {
//   strcpy(_buffer, value ? "ON" : "OFF");
//   drawLabelValue(x, y, label, _buffer);
// }




void OLEDHelper::drawMode(const char* md, int val){
  // drawLabelValue(2, 10, md, (String(val)+"cm3/m").c_str());
  // drawRow(1, (md + String(val, 2)+"cm3/m").c_str());
  // char buffer[20];
  snprintf(_buffer, BUFFER_SIZE, "%s %dcm3/m", md, val);
  const char* cstr = _buffer;
  drawRow(1, cstr); 
}

void OLEDHelper::drawRate(const float val){
  // drawLabelValue(2, 22, "Rate: ", (String(val, 2)+"cm3/min").c_str());
  dtostrf(val, 6, 2, _low_buffer);
  const char* baf = _low_buffer;
  snprintf(_buffer, BUFFER_SIZE, "Rate %scm3/m", baf);
  const char* cstr = _buffer;
  drawRow(2, cstr); 
}

void OLEDHelper::drawPressure(const float val){
  // drawLabelValue(2, 34, "Pressure: ", (String(val, 2)+"MPa").c_str());
  dtostrf(val, 4, 2, _low_buffer);
  snprintf(_buffer, BUFFER_SIZE, "P= %sAtm", _low_buffer);
  // char* c = _buffer;
  // c = strcp(c, "Pressure ");
  // c = itos_d(c, 10, 10, 0);
  // c = strcp(c, "Atm");
  // *c = 0;
  // const char* cstr = _buffer;
  drawRow(3, _buffer); 
}

void OLEDHelper::drawResult(const char* res){
  // drawLabelValue(2, 46, "Result: ", res);
  // snprintf(_buffer, BUFFER_SIZE, "Res: %s", res);
  // const char* cstr = _buffer;
  drawRow(4, res); 
}

void OLEDHelper::drawResultInfo(float rate, float pressure){
  // drawLabelValue(2, 58, ("R: "+String(rate, 2)+".   ").c_str(), ("P: "+String(pressure, 2)).c_str());
}


void OLEDHelper::drawRow(byte pos, const char* value){
  if (pos > 5 || pos < 1) return;
  byte y = 10 + 12 * (pos-1);
  snprintf(_buffer, BUFFER_SIZE, "%s", value);
  _display->drawStr(2, y, _buffer);
}
