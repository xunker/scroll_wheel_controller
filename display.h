// Assume 128x64 oled is connected via I2C
// https://github.com/greiman/SSD1306Ascii
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
// Define proper RST_PIN if required.
#define RST_PIN -1
SSD1306AsciiWire oled;
uint8_t displayHeightInRows;
uint8_t displayWidthInColumns;

// to be called from inside main setup()
void displaySetup() {
  Wire.begin();
  // Wire.setClock(400000L);

  #if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else  // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0

  // oled.setFont(Iain5x7); // proportional
  // oled.setFont(Callibri15); // proportional
  // oled.setFont(Arial14);// proportional
  // oled.setFont(Callibri11_bold);// proportional
  // oled.setFont(TimesNewRoman13);// proportional
  oled.setFont(Adafruit5x7);
  // oled.setFont(fixed_bold10x15);
  // oled.setFont(font5x7);
  // oled.setFont(font8x8);
  // oled.setFont(lcd5x7);
  // oled.setFont(newbasic3x5);
  // oled.setFont(Stang5x7);
  // oled.setFont(System5x7);
  // oled.setFont(Wendy3x5);
  // oled.setFont(X11fixed7x14);
  // oled.setFont(X11fixed7x14B);
  // oled.setFont(ZevvPeep8x16);

  displayHeightInRows = (oled.displayHeight() / oled.fontHeight()) - 1;
  displayWidthInColumns = (oled.displayWidth() / oled.fontWidth()) - 1;

  oled.clear();
  oled.print("Booting");
  for (uint8_t i = 0; i < 10; i++) {
    delay(200);
    oled.print(".");
  }

}

// Print to oled left-justified with newline support
void oledPrintLeftJustify(String msg, uint8_t row)
{
  String substr = "";
  for (uint8_t i = 0; i < msg.length(); i++) {
    if (msg[i] == '\n') {
      oled.setCursor(0, row);
      oled.print(substr);
      substr = "";
      row++;
    } else {
      substr = substr + msg[i];
    }
  }
  if (substr.length() > 0) {
    oled.setCursor(0, row);
    oled.print(substr);
  }
}

// Print to oled right-justified with newline support
void oledPrintRightJustify(String msg, uint8_t row) {
  String substr = "";
  for (uint8_t i = 0; i < msg.length(); i++) {
    if (msg[i] == '\n') {
      oled.setCursor(((oled.displayWidth() - oled.fieldWidth(substr.length()))), row);
      oled.print(substr);
      substr = "";
      row++;
    } else {
      substr = substr + msg[i];
    }
  }
  if (substr.length() > 0) {
    oled.setCursor(((oled.displayWidth() - oled.fieldWidth(substr.length()))), row);
    oled.print(substr);
  }
}

// Print to oled centered with newline support
void oledPrintCentered(String msg, uint8_t row) {
  String substr = "";
  for (uint8_t i = 0; i < msg.length(); i++) {
    if (msg[i] == '\n') {
      oled.setCursor(((oled.displayWidth() - oled.fieldWidth(substr.length()))/2), row);
      oled.print(substr);
      substr = "";
      row++;
    } else {
      substr = substr + msg[i];
    }
  }
  if (substr.length() > 0) {
    oled.setCursor(((oled.displayWidth() - oled.fieldWidth(substr.length()))/2), row);
    oled.print(substr);
  }
}