#ifndef DISPLAY_H
#define DISPLAY_H

#include "config.h"
#include "debugging.h"

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

const char *fontName[] = {
    // "Adafruit5x7",
    // "Iain5x7",         // proportional
    // "Callibri15",      // proportional
    // "Cooper19",        // proportional
    // "Corsiva_12",      // proportional
    // "cp437font8x8",
    // "Arial14",         // proportional
    // "Callibri11_bold", // proportional
    // "TimesNewRoman13", // proportional
    // "fixed_bold10x15",
    // "font5x7",
    "font8x8",
    // "lcd5x7",
    // "newbasic3x5",
    // "Stang5x7",
    // "System5x7",
    // "utf8font10x16", // proportional
    // "Verdana12", // proportional
    // "Wendy3x5",
    // "X11fixed7x14",
    // "X11fixed7x14B",
    // "ZevvPeep8x16"
};

const uint8_t *fontList[] = {
    // Adafruit5x7,
    // Iain5x7,          // proportional
    // Callibri15,       // proportional
    // Cooper19,         // proportional
    // Corsiva_12,       // proportional
    // cp437font8x8,
    // Arial14,          // proportional
    // Callibri11_bold,  // proportional
    // TimesNewRoman13,  // proportional
    // fixed_bold10x15,
    // font5x7,
    font8x8,
    // lcd5x7,
    // newbasic3x5,
    // Stang5x7,
    // System5x7,
    // utf8font10x16, // proportional
    // Verdana12, // proportional
    // Wendy3x5,
    // X11fixed7x14,
    // X11fixed7x14B,
    // ZevvPeep8x16
};

uint8_t nFont = sizeof(fontList) / sizeof(uint8_t *);
#define DEFAULT_FONT 0
uint8_t currentFont = 0;

void setFont(uint8_t fontNumber) {
  oled.setFont(fontList[fontNumber]);
  // displayHeightInRows = (oled.displayHeight() / oled.fontHeight()) - 1;

  displayWidthInColumns = (oled.displayWidth() / oled.fontWidth()) - 1;
  // displayWidthInColumns = oled.displayWidth();
  displayHeightInRows = oled.displayRows();
}

// to be called from inside main setup()
void displaySetup() {
  Wire.begin();
  // Wire.setClock(400000L);

  #if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else  // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0

  setFont(currentFont);

  oled.clear();
  oled.print(F("Display\nInitialized"));
}

// Print to oled left-justified with newline support
void oledPrintLeftJustify(char * msg, uint8_t row)
{
  debugf("oledPrintLeftJustify: '");
  debug(msg);
  debugfln("'");
  String substr = "";
  for (uint8_t i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n') {
      oled.setCursor(0, row);
      oled.print(substr);
      substr = "";
      row += oled.fontRows();
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
void oledPrintRightJustify(char * msg, uint8_t row) {
  debugf("oledPrintRightJustify: '");
  debug(msg);
  debugfln("'");
  String substr = "";
  for (uint8_t i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n') {
      int8_t col = ((oled.displayWidth() - oled.fieldWidth(substr.length()))); // signed!
      oled.setCursor(col < 0 ? 0 : col, row);
      oled.print(substr);
      substr = "";
      row += oled.fontRows();
    } else {
      substr = substr + msg[i];
    }
  }
  if (substr.length() > 0) {
    int8_t col = ((oled.displayWidth() - oled.fieldWidth(substr.length()))); // signed!
    oled.setCursor(col < 0 ? 0 : col, row);
    oled.print(substr);
  }
}

// Print to oled centered with newline support
void oledPrintCentered(char * msg, uint8_t row) {
  debugf("oledPrintCentered: '");
  debug(msg);
  debugfln(")");
  String substr = "";
  for (uint8_t i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n') {
      int8_t col = ((oled.displayWidth() - oled.fieldWidth(substr.length())) / 2); // signed!

      oled.setCursor(col < 0 ? 0 : col, row);
      oled.print(substr);
      substr = "";
      row += oled.fontRows();
    } else {
      substr = substr + msg[i];
    }
  }
  if (substr.length() > 0) {
    int8_t col = ((oled.displayWidth() - oled.fieldWidth(substr.length())) / 2); // signed!
    oled.setCursor(col < 0 ? 0 : col, row);
    oled.print(substr);
  }
}

#endif
