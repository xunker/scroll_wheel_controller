/*

Scroll-wheel USB media controller
http://github.com/xunker/scroll_wheel_controller

Hardware:
  * Arduino-compatible MCU with USB-HID capability. Tested with ATmega32u4-based
    Sparkfun Pro Micro.
  * 128x64 I2C OLED display
  * Rotary encoder with momentary centre button
  * 4 buttons in each cardinal direction (up, down, left, right)

Required libraries:
  * Arduino HID Project: https://github.com/NicoHood/HID/
  * SSD1306: https://github.com/greiman/SSD1306Ascii
  * Rotary: https://github.com/brianlow/Rotary

*/

// Config options for specific microcontrollers
#include "mcu.h"

// https://github.com/NicoHood/HID/blob/master/examples/Consumer/Consumer.ino
// https://github.com/NicoHood/HID/blob/cc946b9dd7b73b61293a4aa74444de3a5a871bb0/src/KeyboardLayouts/ImprovedKeylayoutsUS.h
// https://github.com/NicoHood/HID/blob/d4938ddcff7970bc1d32a040a08afeac4915e4a9/src/HID-APIs/ConsumerAPI.h
#include "HID-Project.h"

#include "config.h"
#include "buttons.h"
#include "encoder.h"
#include "display.h"
#include "debugging.h"
#include "layouts.h"

#define KEYBOARD_HID_TYPE 0
#define CONSUMER_HID_TYPE 1
#define MOUSE_HID_TYPE 2
struct actionKeypress {
  const uint8_t hidType; // KEYBOARD_HID_TYPE,  CONSUMER_HID_TYPE, MOUSE_HID_TYPE
  const uint16_t keyCode; // ConsumerKeycode is uint16t, KeyboardKeycode is uint8_t
};

struct controlAction {
  const char name[MAX_LABEL_LENGTH]; // name of action
  actionKeypress keys[MAX_KEYS_PER_ACTION]; // standard keys to send
  uint8_t modeMask;
};

/*
modeMask - binary bitmask
0b0000000x - Key down time
  0: default key down time
  1: long key down time

combine modeMasks using |
*/

#define LONG_KEY_DOWN_TIME 0b00000001

#define MOUSE_EVENT 0b10000000
#define MOUSE_SCROLL_POSITIVE MOUSE_EVENT | 0b01000000
#define MOUSE_SCROLL_NEGATIVE MOUSE_EVENT | 0b00100000
#define MOUSE_LEFT_CLICK MOUSE_EVENT | 0b00010000
#define MOUSE_RIGHT_CLICK MOUSE_EVENT | 0b00001000
#define MOUSE_MIDDLE_CLICK MOUSE_EVENT | 0b00000100

struct controlMode {
  const char name[MAX_LABEL_LENGTH];
  const char wheelName[MAX_LABEL_LENGTH]; // name of scroll wheel action
  controlAction left;
  controlAction right;
  controlAction wheelCW;
  controlAction wheelCCW;
  controlAction middle;
};

#include "control_modes.h"

const uint8_t numberOfModes = sizeof (controlModeList) / sizeof (controlModeList[0]);

uint8_t currentModeIndex = (DEFAULT_MODE > (numberOfModes - 1)) ? 0 : DEFAULT_MODE;

uint8_t previousModeIndex = currentModeIndex;

// a mode that can be quickly toggled between, ususally volume control
uint8_t toggleModeIndex = 0;

// when did the user last perform an action?
unsigned long lastAction = 0;

// Screensaver state
bool screensaverEnabled = false;
const char screensaverText[] = SCREENSAVER_TEXT;

/* If ENABLE_DEBUGGING is on, this keeps track of the last time the current
   state was reported on the Serial port. If screensaver mode is enabled, this
   is also the last time the the screensaver text was moved. Milliseconds */
unsigned long nextOutput = OUTPUT_EVERY;

boolean keyboardPressed = false; // is Keyboard in currently-pressed state?
boolean consumerPressed = false; // is Consumer in currently-pressed state?

controlMode currentMode() {
  return controlModeList[currentModeIndex];
}

controlMode toggleMode() {
  return controlModeList[toggleModeIndex];
}

controlMode previousMode() {
  return controlModeList[previousModeIndex];
}

// Update the connected oled display
void updateDisplay() {
  oled.clear();

  oledPrintCentered(currentMode().name, 0);

  // oled.setCursor(0, 1);
  // oled.print("01234567890123456789012345678901234567890");

  // middle button
  if (strlen(currentMode().middle.name) > 0)
    oledPrintCentered(currentMode().middle.name, currentLayout().middleButtonLabelRow);

  // left button
  if (strlen(currentMode().left.name) > 0)
    oledPrintLeftJustify(currentMode().left.name, currentLayout().leftRightButtonLabelRow);

  // right button
  if (strlen(currentMode().right.name) > 0)
    oledPrintRightJustify(currentMode().right.name, currentLayout().leftRightButtonLabelRow);

  // wheel actions
  if (strlen(currentMode().wheelName) > 0) {
    const char label[MAX_LABEL_LENGTH*2];
    strcpy(label, currentMode().wheelCW.name);
    strcat(label, " ");
    strcat(label, currentMode().wheelName);
    strcat(label, " ");
    strcat(label, currentMode().wheelCCW.name);

    oledPrintCentered(label, currentLayout().wheelActionLabelRow);
  }

  // mode quick-toggle
  const char quickToggleLabel[MAX_LABEL_LENGTH*2];
  if (previousModeIndex == currentModeIndex) {
    if (currentModeIndex != toggleModeIndex) {
      strcpy(quickToggleLabel, toggleMode().name);
      strcat(quickToggleLabel, " ->");

    } else {
      strcpy(quickToggleLabel, "--");
    }
  } else {
    if (previousModeIndex != toggleModeIndex) {
      strcpy(quickToggleLabel, "<- ");
      strcat(quickToggleLabel, previousMode().name);
    } else {
      strcpy(quickToggleLabel, toggleMode().name);
      strcat(quickToggleLabel, " ->");
    }
  }
  if (currentLayout().quickToggleLabelRow <= displayHeightInRows-1)
    oledPrintCentered(quickToggleLabel, currentLayout().quickToggleLabelRow);
}

/* Are we currently in quick-toggle mode? */
bool inToggleMode() {
  return ((previousModeIndex != currentModeIndex) && (previousModeIndex != toggleModeIndex));
}

/* record the last time an action was performed, and disable the screensaver
   if it is currently active */
void updateLastAction() {
  lastAction = millis();
  if (screensaverEnabled) {
    screensaverEnabled = false;
    updateDisplay();
  }
}

void setup() {
  #ifdef ENABLE_DEBUGGING
    Serial.begin(DEBUG_BAUD);
  #endif

  mcuSetup();

  displaySetup();

  layoutSetup();

  buttonsSetup();

  // Put this in main setup() to give you a chance to reprogram the MCU in case
  // things get wedged; just hold th middle button while booting and the MCU
  // Will enable the serial port and then just wait forever
  middleButton.read();
  if (middleButton.isPressed()) {
    Serial.begin(9600);
    oled.clear();
    oled.print(F("Waiting for\nProgrammer\n"));
    uint8_t dotCounter = 0;
    while (true) {
      Serial.print(millis());
      Serial.println(F(" Programming Mode - Waiting for Programmer..."));

      if (dotCounter++ > 10) {
        dotCounter = 0;
        oled.clearField(0, 2, 10);
      }
      oled.print(".");
      delay(1000);
    }
  }

  updateDisplay();

  Keyboard.begin();
  Consumer.begin();
  Mouse.begin();

  encoderSetup();
}

// Send action but don't release the keys
void sendAction(controlAction actionToSend)
{
  setIndicatorLed(1);
  updateLastAction();

  debugf("Sending key action '");
  debug(actionToSend.name);
  debugfln("'");

  for (uint8_t i = 0; i < MAX_KEYS_PER_ACTION; i++) {
    if (actionToSend.keys[i].keyCode != NULL) {

      debugf("HID_TYPE: ");
      debugfmt(actionToSend.keys[i].keyCode, HEX);

      if (actionToSend.keys[i].hidType == CONSUMER_HID_TYPE) {
        debugfln(" CONSUMER_HID_TYPE");

        Consumer.press(actionToSend.keys[i].keyCode);
        consumerPressed = true;
      } else if (actionToSend.keys[i].hidType == MOUSE_HID_TYPE) {
        debugfln(" MOUSE_HID_TYPE");

        if (bitRead(actionToSend.keys[i].keyCode, 6)) {
          debugfln("scrolling down");
          Mouse.move(0, 0, MOUSE_SCROLL_AMOUNT);
        } else if (bitRead(actionToSend.keys[i].keyCode, 5)) {
          debugfln("scrolling up");
          Mouse.move(0, 0, -MOUSE_SCROLL_AMOUNT);
        } else if (bitRead(actionToSend.keys[i].keyCode, 4)) {
          debugfln("left click");
          Mouse.click(MOUSE_LEFT);
        } else if (bitRead(actionToSend.keys[i].keyCode, 3)) {
          debugfln("right click");
          Mouse.click(MOUSE_RIGHT);
        } else if (bitRead(actionToSend.keys[i].keyCode, 2)) {
          debugfln("middle click");
          Mouse.click(MOUSE_MIDDLE);
        }
      } else {
        debugfln(" KEYBOARD_HID_TYPE");

        Keyboard.press((KeyboardKeycode)actionToSend.keys[i].keyCode);
        keyboardPressed = true;
      }

    }
  }
}

void releaseAction(controlAction actionToRelease) {
  setIndicatorLed(0);

  debugf("Releasing key action '");
  debug(actionToRelease.name);
  debugfln("'");

  if (keyboardPressed) {
    Keyboard.releaseAll();
    keyboardPressed = false;
  }

  if (consumerPressed) {
    Consumer.releaseAll();
    consumerPressed = false;
  }
}

// send action and release the keys immediately after correct delay
void sendActionAndRelease(controlAction actionToSend) {

  sendAction(actionToSend);

  if (actionToSend.modeMask & LONG_KEY_DOWN_TIME) {
    debugfln("Using KEY_DOWN_TIME_LONG");
    delay(KEY_DOWN_TIME_LONG);
  } else {
    delay(KEY_DOWN_TIME_REGULAR);

  }

  releaseAction(actionToSend);
}

void changeModeMessage() {
  debugf("Changing control mode to ");
  debug(currentModeIndex);
  debugf(" '");
  debug(currentMode().name);
  debugfln("'");
}

void returnToPreviousMode() {
  debugfln("Returning to previous mode");
  currentModeIndex = previousModeIndex;
  changeModeMessage();
  updateDisplay();
}

void toggleToggleMode() {
  if (currentModeIndex == toggleModeIndex) {
    if (previousModeIndex != toggleModeIndex) {
      returnToPreviousMode();
    } else {
      debugfln("Toggle mode and previous mode are the same");
    }
  } else {
    debugf("Temporary toggle to '");
    debug(toggleMode().name);
    debugfln("'");

    currentModeIndex = toggleModeIndex;
    updateDisplay();
  }
}

void loop() {
  readButtons();

  unsigned long currentMillis = millis();

  if (nextOutput < currentMillis) {
    nextOutput = currentMillis + OUTPUT_EVERY;

    debug(currentMillis);
    debugf(" Current Control Mode is '");
    debug(currentMode().name);
    debugfln("'");

    // updateDisplay();

    if (!screensaverEnabled && (lastAction > SCREENSAVER_STARTS_IN) && (lastAction < currentMillis - SCREENSAVER_STARTS_IN)) {
      screensaverEnabled = true;
    }

    if (screensaverEnabled) {
      oled.clear();
      oled.setCursor(random(0, oled.displayWidth()-oled.strWidth(screensaverText)), random(0, displayHeightInRows));
      oled.print(screensaverText);
    }
  }

  if (upButton.isPressed() && middleButton.wasPressed()) {
    nextLayout();

    oled.clear();
    oled.print(F("Font: "));
    oled.print(currentLayout().fontName);
    delay(1000);
    updateDisplay();

  } else if (leftButton.wasPressed()) {
    sendAction(currentMode().left);
  } else if (leftButton.wasReleased()) {
    releaseAction(currentMode().left);
  } else if (rightButton.wasPressed()) {
    sendAction(currentMode().right);
  } else if (rightButton.wasReleased()) {
    releaseAction(currentMode().right);
  } else if (middleButton.wasPressed()) {
    sendAction(currentMode().middle);
  } else if (middleButton.wasReleased()) {
    releaseAction(currentMode().middle);

  } else if (upButton.wasPressed()) {
    updateLastAction();

    currentModeIndex++;
    if (currentModeIndex >= numberOfModes) {
      currentModeIndex = 0;
    }
    previousModeIndex = currentModeIndex;

    changeModeMessage();
    updateDisplay();
  } else if (downButton.wasPressed()) {
    updateLastAction();

    toggleToggleMode();
  }

  if (encoderTurned < 0) {
    debugf("CCW: ");
    debugln(encoderTurned);
    encoderTurned++;
    sendActionAndRelease(currentMode().wheelCCW);
  } else if (encoderTurned > 0) {
    debugf("CW: ");
    debugln(encoderTurned);
    encoderTurned--;
    sendActionAndRelease(currentMode().wheelCW);
  }

  if (inToggleMode() && (lastAction < currentMillis - TOGGLE_MODE_EXPIRES_IN)) {
    debugfln("Toggle Mode expired; Returning to previous mode");
    returnToPreviousMode();
  }
}
