#ifndef LAYOUT_H
#define LAYOUT_H

#include "display.h" // specific config and functions for the display used

/*
A Layout defines:
  * The font to use
  * The row positions of various elements
*/

struct displayLayout {
  const uint8_t* font; // Pointer to actual font
  const char fontName[16]; // Name of font
  const uint8_t currentModeLabelRow;
  const uint8_t leftRightButtonLabelRow;
  const uint8_t middleButtonLabelRow;
  const uint8_t wheelActionLabelRow;
  const uint8_t quickToggleLabelRow;
};

/*

To add fonts or change the positon of labels on the screen, you edit the
`displayLayouts` array. Elements in this array are structured as:

  {font, fontName, currentModeLabelRow, leftRightButtonLabelRow, middleButtonLabelRow, wheelActionLabelRow, quickToggleLabelRow}

`font`: Can be any font included in the SSD1306 library (see the list at
https://github.com/greiman/SSD1306Ascii/tree/master/src/fonts). Examples:
  * Adafruit5x7
  * Arial14          // proportional
  * Arial_bold_14    // proportional
  * Callibri11_bold  // proportional
  * Callibri15       // proportional
  * Corsiva_12       // proportional
  * cp437font8x8
  * fixed_bold10x15
  * font5x7
  * font8x8
  * Iain5x7          // proportional
  * lcd5x7
  * newbasic3x5
  * Stang5x7
  * System5x7
  * TimesNewRoman13  // proportional
  * TimesNewRoman16  // proportional
  * utf8font10x16    // proportional
  * Verdana12        // proportional
  * Wendy3x5
  * X11fixed7x14
  * X11fixed7x14B
  * ZevvPeep8x16

`fontName`: The human-readable name of the font as a character array. Limited to 16 characters

`currentModeLabelRow`: The row where the "current mode" label will appear
`leftRightButtonLabelRow`: The row where the left and right button labels will appear
`middleButtonLabelRow`: The row where the middle button label will appear
`wheelActionLabelRow`: The row where the encoder/scroll-wheel action label will appear
`quickToggleLabelRow`: The row where the quick-toggle label will appear

Note on "row" nomenclature above: the SSD1306 display allocated the display in
rows of 8 pixels high. This means a 128x64 OLED has 8 rows. All of the "row"
defintions below assume you are referring to one of the 8-pixel-high rows.
*/

#include "fonts/font8x8_custom.h"
const displayLayout displayLayouts[] = {
  {font8x8_custom, "font8x8", 0, 2, 2, 5, 7},
  {Arial14, "Arial14", 0, 2, 2, 6, 8}, // 8 is off screen, won't be shown
};

const uint8_t numberOfLayouts = sizeof (displayLayouts) / sizeof (displayLayouts[0]);

uint8_t currentLayoutIndex = 0;

const displayLayout currentLayout() {
  return displayLayouts[currentLayoutIndex];
}

void setLayout(uint8_t layoutIndex) {
  currentLayoutIndex = layoutIndex;


  oled.setFont(currentLayout().font);

  displayWidthInColumns = (oled.displayWidth() / oled.fontWidth()) - 1;
  displayHeightInRows = oled.displayRows();
}

void nextLayout() {
  currentLayoutIndex += 1;

  if (currentLayoutIndex >= numberOfLayouts)
      currentLayoutIndex = 0;

  setLayout(currentLayoutIndex);
}

// to be called from inside main setup()
void layoutSetup() {
  setLayout(currentLayoutIndex);
}

#endif
