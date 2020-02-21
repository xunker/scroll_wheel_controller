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

// to be called from inside main setup()
void displaySetup() {
  Wire.begin();
  // Wire.setClock(400000L);

  #if RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS, RST_PIN);
  #else  // RST_PIN >= 0
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
  #endif // RST_PIN >= 0

  oled.clear();
  oled.print(F("Display\nInitialized"));
}

void appendCharToArray(char * charArray, char aChar) {
  /* strcat needs two char arrays, so we build one with the current char
      https://stackoverflow.com/posts/22429675/revisions */
  char cToStr[2] { aChar, '\0' };
  strcat(charArray, cToStr);
}

// Print to oled left-justified with newline support
void oledPrintLeftJustify(char * msg, uint8_t row)
{
  debugf("oledPrintLeftJustify: '");
  debug(msg);
  debugfln("'");

  char substr[MAX_LABEL_LENGTH*2] = "";
  for (uint8_t i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n') {
      oled.setCursor(0, row);
      oled.print(substr);

      debugf(":");
      debugln(substr);

      strcpy(substr, ""); // empty the temporary string

      row += oled.fontRows();
    } else {
      appendCharToArray(substr, msg[i]);
    }
  }
  if (strlen(substr) > 0) {
    oled.setCursor(0, row);
    oled.print(substr);

    debugf(":");
    debugln(substr);
  }
}

// Print to oled right-justified with newline support
void oledPrintRightJustify(char * msg, uint8_t row) {
  debugf("oledPrintRightJustify: '");
  debug(msg);
  debugfln("'");

  char substr[MAX_LABEL_LENGTH*2] = "";
  for (uint8_t i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n') {
      int8_t col = ((oled.displayWidth() - oled.strWidth(substr))); // signed!

      oled.setCursor(col < 0 ? 0 : col, row);
      oled.print(substr);

      debug(substr);
      debugfln(":");

      strcpy(substr, ""); // empty the temporary string

      row += oled.fontRows();
    } else {
      appendCharToArray(substr, msg[i]);
    }
  }
  if (strlen(substr) > 0) {
    int8_t col = ((oled.displayWidth() - oled.strWidth(substr))); // signed!
    oled.setCursor(col < 0 ? 0 : col, row);
    oled.print(substr);

    debug(substr);
    debugfln(":");
  }
}

// Print to oled centered with newline support
void oledPrintCentered(char * msg, uint8_t row) {
  debugf("oledPrintCentered: '");
  debug(msg);
  debugfln("'");

  char substr[MAX_LABEL_LENGTH*2] = "";
  for (uint8_t i = 0; i < strlen(msg); i++) {
    if (msg[i] == '\n') {
      int8_t col = ((oled.displayWidth() - oled.strWidth(substr)) / 2); // signed!

      oled.setCursor(col < 0 ? 0 : col, row);
      oled.print(substr);

      debugf(":");
      debug(substr);
      debugfln(":");

      strcpy(substr, ""); // empty the temporary string

      row += oled.fontRows();
    } else {
      appendCharToArray(substr, msg[i]);
    }
  }
  if (strlen(substr) > 0) {
    int8_t col = ((oled.displayWidth() - oled.strWidth(substr)) / 2); // signed!
    oled.setCursor(col < 0 ? 0 : col, row);
    oled.print(substr);

    debugf(":");
    debug(substr);
    debugfln(":");
  }
}

#endif
