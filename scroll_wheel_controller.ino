// Config options for specific microcontrollers
#include "mcu.h"

// https://github.com/NicoHood/HID/blob/master/examples/Consumer/Consumer.ino
// https://github.com/NicoHood/HID/blob/cc946b9dd7b73b61293a4aa74444de3a5a871bb0/src/KeyboardLayouts/ImprovedKeylayoutsUS.h
// https://github.com/NicoHood/HID/blob/d4938ddcff7970bc1d32a040a08afeac4915e4a9/src/HID-APIs/ConsumerAPI.h
#include "HID-Project.h"

#include "buttons.h"

#include "encoder.h"

#include "display.h"

#define USE_SERIAL
#ifdef USE_SERIAL
  #define debug(msg) Serial.print(msg)
  #define debugln(msg) Serial.println(msg)
  #define debuglnfmt(msg, fmt) Serial.println(msg, fmt)
#else
  #define debug(msg)
  #define debugln(msg)
  #define debuglnfmt(msg, fmt)
#endif

#define MOUSE_SCROLL_AMOUNT 5

#define MAX_KEYS_PER_ACTION 3
struct controlAction {
  const String name; // name of action
  KeyboardKeycode keys[MAX_KEYS_PER_ACTION]; // standard keys to send
  ConsumerKeycode consumerKey; // consumer key to send (just one)
  uint8_t modeMask;
};

/*
modeMask - binary bitmask
0b0000000x - Key down time
  0: default key down time
  1: long key down time

0b10000000 - action will also send mouse events
0b11000000 - mouse scroll up
0b10100000 - mouse scroll down
0b10010000 - mouse left click
0b10001000 - mouse right click
0b10000100 - mouse middle click

combine modeMasks using |
*/

#define LONG_KEY_DOWN_TIME 0b00000001

#define MOUSE_EVENT 0b10000000
#define MOUSE_SCROLL_POSITIVE MOUSE_EVENT | 0b01000000
#define MOUSE_SCROLL_NEGATIVE MOUSE_EVENT | 0b00100000
#define MOUSE_LEFT_CLICK MOUSE_EVENT | 0b00010000
#define MOUSE_RIGHT_CLICK MOUSE_EVENT | 0b00001000
#define MOUSE_MIDDLE_CLICK MOUSE_EVENT | 0b00000100

#define KEY_DOWN_TIME_REGULAR 50 // milliseconds
#define KEY_DOWN_TIME_LONG 1100  // milliseconds

struct controlMode {
  const String name;
  controlAction left;
  controlAction right;
  controlAction wheelCW;
  controlAction wheelCCW;
  controlAction middle;
};

#define NUMBER_OF_MODES 5
/*

Ideas for other control modes:
  CONSUMER_BRIGHTNESS_UP
  CONSUMER_BRIGHTNESS_DOWN
  CONSUMER_SCREENSAVER
  CONSUMER_POWER
  CONSUMER_SLEEP
  HID_CONSUMER_RESET

  Switch application and application windows

Option to define/switch "control sets", e.g. one set for Macos and one for
Windows, or for specific apps. Do this by long-pressing the mode select button.

When on toggle mode, automatically return to previous mode if no actions after
N seconds.

Encoder wheel acceleration. E.g. in VLC mode, use faster scrub keypress the
longer I continuously rotate the wheel.

Screen saver/screen blanking on inactivity

*/
controlMode controlModeList[NUMBER_OF_MODES] = {
    {{"Volume"},
     {"--", NULL, NULL, NULL, NULL},
     {"--", NULL, NULL, NULL, NULL},
     {"- Vol", NULL, NULL, NULL, MEDIA_VOLUME_DOWN},
     {"Vol +", NULL, NULL, NULL, MEDIA_VOLUME_UP},
     {"Mute", NULL, NULL, NULL, MEDIA_VOLUME_MUTE}},

    {{"Media"},
     {"Prev\nTrack", NULL, NULL, NULL, MEDIA_PREVIOUS},
     {"Next\nTrack", NULL, NULL, NULL, MEDIA_NEXT},
     {"< Seek", NULL, NULL, NULL, MEDIA_REWIND, LONG_KEY_DOWN_TIME},
     {"Seek >", NULL, NULL, NULL, MEDIA_FAST_FORWARD, LONG_KEY_DOWN_TIME},
     {"Play/\nPause", NULL, NULL, NULL, MEDIA_PLAY_PAUSE}},

    {{"VLC"},
     {"Prev\nTrack", KEY_LEFT_GUI, KEY_LEFT_ARROW, NULL, NULL},
     {"Next\nTrack", KEY_LEFT_GUI, KEY_RIGHT_ARROW, NULL, NULL},
     {"< Scrub", KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_LEFT_ARROW, NULL},
     {"Scrub >", KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_RIGHT_ARROW, NULL},
     {"Play/\nPause", KEY_SPACE, NULL, NULL, NULL}},

    {{"Mouse"},
     {"Left\nClick", NULL, NULL, NULL, NULL, MOUSE_LEFT_CLICK},
     {"Right\nClick", NULL, NULL, NULL, NULL, MOUSE_RIGHT_CLICK},
     {"^ Scroll", NULL, NULL, NULL, NULL, MOUSE_SCROLL_NEGATIVE},
     {"Scroll v", NULL, NULL, NULL, NULL, MOUSE_SCROLL_POSITIVE},
     {"Middle\nClick", NULL, NULL, NULL, NULL, MOUSE_MIDDLE_CLICK}},

    {{"Navigation"},
     {"Next\nPage", KEY_LEFT_GUI, KEY_LEFT_BRACE, NULL, NULL},  // CONSUMER_BROWSER_BACK maybe
     {"Prev\nPage", KEY_LEFT_GUI, KEY_RIGHT_BRACE, NULL, NULL}, // CONSUMER_BROWSER_FORWARD maybe
     {"^ Arrow", KEY_UP_ARROW, NULL, NULL, NULL},
     {"Arrow v", KEY_DOWN_ARROW, NULL, NULL, NULL},
     {"Enter", KEY_ENTER, NULL, NULL, NULL}}};

uint8_t currentModeIndex = 0;

uint8_t previousModeIndex = currentModeIndex;

// a mode that can be quickly toggled between, ususally volume control
uint8_t toggleModeIndex = 0;

// when did the user last perform an action?
unsigned long lastAction = 0;
// If in toggle mode and no action performed, return to previous mode in this many milliseconds
#define TOGGLE_MODE_EXPIRES_IN 10000 // ms

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

  oledPrintCentered(currentMode().name + " Mode", 0);

  // oled.setCursor(0, 1);
  // oled.print("01234567890123456789012345678901234567890");

  // middle button
  oledPrintCentered(currentMode().middle.name, 2);

  // left button
  oledPrintLeftJustify(currentMode().left.name, 2);

  // right button
  oledPrintRightJustify(currentMode().right.name, 2);

  // wheel actions
  oledPrintCentered((currentMode().wheelCW.name +  " .. " + currentMode().wheelCCW.name), displayHeightInRows - 3);

  // mode quick-toggle
  if (previousModeIndex == currentModeIndex) {
    if (currentModeIndex != toggleModeIndex) {
      oledPrintCentered(toggleMode().name + " ->", displayHeightInRows - 1);
    } else {
      oledPrintCentered("--", displayHeightInRows - 1);
    }
  } else {
    if (previousModeIndex != toggleModeIndex) {
      oledPrintCentered("<- " + previousMode().name, displayHeightInRows - 1);
    } else {
      oledPrintCentered(toggleMode().name + " ->", displayHeightInRows - 1);
    }
  }
}

bool inToggleMode() {
  return ((previousModeIndex != currentModeIndex) && (previousModeIndex != toggleModeIndex));
}

void updateLastAction() {
  lastAction = millis();
}

void setup() {
  // #ifdef USE_SERIAL
    // leave serial enabled so we can reprogram properly
    Serial.begin(9600);
  // #endif

  displaySetup();

  updateDisplay();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);

  buttonsSetup();

  Keyboard.begin();
  Mouse.begin();

  encoderSetup();
}

#define OUTPUT_EVERY 5000
unsigned long nextOutput = OUTPUT_EVERY;

// Send action but don't release the keys
void sendAction(controlAction actionToSend)
{
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ON);
  updateLastAction();

  debug("Sending key action '");
  debug(actionToSend.name);
  debugln("'");

  for (uint8_t i = 0; i < MAX_KEYS_PER_ACTION; i++) {
    if (actionToSend.keys[i]) {
      debuglnfmt(actionToSend.keys[i], HEX);
      Keyboard.press(actionToSend.keys[i]);
    }
  }

  if (actionToSend.consumerKey) {
    debuglnfmt(actionToSend.consumerKey, HEX);
    Keyboard.press(actionToSend.consumerKey);
  }

  if (bitRead(actionToSend.modeMask, 7)) {
    if (bitRead(actionToSend.modeMask, 6)) {
      debugln("scrolling down");
      Mouse.move(0, 0, MOUSE_SCROLL_AMOUNT);
    }
    if (bitRead(actionToSend.modeMask, 5)) {
      debugln("scrolling up");
      Mouse.move(0, 0, -MOUSE_SCROLL_AMOUNT);
    }
    if (bitRead(actionToSend.modeMask, 4)) {
      debugln("left click");
      Mouse.click(MOUSE_LEFT);
    }
    if (bitRead(actionToSend.modeMask, 3)) {
      debugln("right click");
      Mouse.click(MOUSE_RIGHT);
    }
    if (bitRead(actionToSend.modeMask, 2)) {
      debugln("middle click");
      Mouse.click(MOUSE_MIDDLE);
    }
  }
}

void releaseAction(controlAction actionToRelease) {
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);

  debug("Releasing key action '");
  debug(actionToRelease.name);
  debugln("'");

  Keyboard.releaseAll();
}

// send action and release the keys immediately after correct delay
void sendActionAndRelease(controlAction actionToSend) {

  sendAction(actionToSend);

  if (actionToSend.modeMask & LONG_KEY_DOWN_TIME) {
    debugln("Using KEY_DOWN_TIME_LONG");
    delay(KEY_DOWN_TIME_LONG);
  } else {
    delay(KEY_DOWN_TIME_REGULAR);

  }

  releaseAction(actionToSend);
}

void changeModeMessage() {
  debug("Changing control mode to ");
  debug(currentModeIndex);
  debug(" '");
  debug(currentMode().name);
  debugln("'");
}

void loop() {
  readButtons();
  
  unsigned long currentMillis = millis();

  if (nextOutput < currentMillis) {
    nextOutput = currentMillis + OUTPUT_EVERY;

    debug(currentMillis);
    debug(" Current Control Mode is '");
    debug(currentMode().name);
    debugln("'");

    // updateDisplay();
  }

  if (leftButton.wasPressed()) {
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
    if (currentModeIndex >= NUMBER_OF_MODES) {
      currentModeIndex = 0;
    }
    previousModeIndex = currentModeIndex;

    changeModeMessage();
    updateDisplay();
  } else if (downButton.wasPressed()) {
    updateLastAction();

    if (currentModeIndex == toggleModeIndex) {
      if (previousModeIndex != toggleModeIndex) {
        debugln("Returning to previous mode");
        currentModeIndex = previousModeIndex;
        changeModeMessage();
        updateDisplay();
      } else {
        debugln("Toggle mode and previous mode are the same");
      }
    } else {
      debug("Temporary toggle to '");
      debug(toggleMode().name);
      debugln("'");

      currentModeIndex = toggleModeIndex;
      updateDisplay();
    }
  }

  if (encoderTurnedCCW) {
    sendActionAndRelease(currentMode().wheelCCW);
  } else if (encoderTurnedCW) {
    sendActionAndRelease(currentMode().wheelCW);
  }

  if (encoderTurnedCW || encoderTurnedCCW) {
    encoderTurnedCCW = false;
    encoderTurnedCW = false;
  }

  if (inToggleMode() && (lastAction < currentMillis - TOGGLE_MODE_EXPIRES_IN)) {
    debugln("Toggle Mode expired; Returning to previous mode");
    currentModeIndex = previousModeIndex;
    changeModeMessage();
    updateDisplay();
  }
}
