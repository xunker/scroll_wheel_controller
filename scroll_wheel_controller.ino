// Config options for specific microcontrollers
#include "mcu.h"

// https://github.com/NicoHood/HID/blob/master/examples/Consumer/Consumer.ino
// https://github.com/NicoHood/HID/blob/cc946b9dd7b73b61293a4aa74444de3a5a871bb0/src/KeyboardLayouts/ImprovedKeylayoutsUS.h
// https://github.com/NicoHood/HID/blob/d4938ddcff7970bc1d32a040a08afeac4915e4a9/src/HID-APIs/ConsumerAPI.h
#include "HID-Project.h"

#include "buttons.h"

#include "encoder.h"

#include "display.h"

// #define ENABLE_DEBUGGING
#ifdef ENABLE_DEBUGGING
  #define debugf(msg) Serial.print(F(msg))
  #define debug(msg) Serial.print(msg)
  #define debugfln(msg) Serial.println(F(msg))
  #define debugln(msg) Serial.println(msg)
  #define debuglnfmt(msg, fmt) Serial.println(msg, fmt)
#else
  #define debugf(msg)
  #define debug(msg)
  #define debugfmt(msg, fmt)
  #define debugln(msg)
  #define debugfln(msg)
  #define debuglnfmt(msg, fmt)
#endif

#define MOUSE_SCROLL_AMOUNT 5

#define KEYBOARD_HID_TYPE 0
#define CONSUMER_HID_TYPE 1
#define MOUSE_HID_TYPE 1
struct actionKeypress {
  const uint8_t hidType; // KEYBOARD_HID_TYPE,  CONSUMER_HID_TYPE, MOUSE_HID_TYPE
  const uint16_t keyCode; // ConsumerKeycode is uint16t, KeyboardKeycode is uint8_t
};

#define MAX_KEYS_PER_ACTION 3
struct controlAction {
  const String name; // name of action
  // KeyboardKeycode keys[MAX_KEYS_PER_ACTION]; // standard keys to send
  // ConsumerKeycode consumerKey; // consumer key to send (just one)
  actionKeypress keys[MAX_KEYS_PER_ACTION]; // standard keys to send
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

#define KEY_DOWN_TIME_REGULAR 10 // milliseconds
#define KEY_DOWN_TIME_LONG 1100  // milliseconds

struct controlMode {
  const String name;
  const String wheelName; // name of scroll wheel action
  controlAction left;
  controlAction right;
  controlAction wheelCW;
  controlAction wheelCCW;
  controlAction middle;
};

/* HID consumer/media key abstraction for OS compatibility */

#define VOLUME_UP_CODE MEDIA_VOLUME_UP
// #define VOLUME_UP_CODE HID_CONSUMER_VOLUME_INCREMENT
#define VOLUME_DOWN_CODE MEDIA_VOLUME_DOWN
// #define VOLUME_DOWN_CODE HID_CONSUMER_VOLUME_DECREMENT
#define VOLUME_MUTE_CODE MEDIA_VOLUME_MUTE
// #define VOLUME_MUTE_CODE HID_CONSUMER_MUTE

#define PLAY_PAUSE_CODE MEDIA_PLAY_PAUSE
// #define PLAY_PAUSE_CODE HID_CONSUMER_PLAY

// #define TRACK_NEXT_CODE MEDIA_NEXT
// #define TRACK_NEXT_CODE HID_CONSUMER_FAST_FORWARD
#define TRACK_NEXT_CODE HID_CONSUMER_SCAN_NEXT_TRACK
// #define TRACK_PREVIOUS_CODE MEDIA_PREVIOUS
// #define TRACK_PREVIOUS_CODE HID_CONSUMER_REWIND
#define TRACK_PREVIOUS_CODE HID_CONSUMER_SCAN_PREVIOUS_TRACK

// #define TRACK_SCAN_FORWARD MEDIA_FAST_FORWARD
#define TRACK_SCAN_FORWARD HID_CONSUMER_SCAN_NEXT_TRACK
// #define TRACK_SCAN_BACKWARD MEDIA_REWIND
#define TRACK_SCAN_BACKWARD HID_CONSUMER_SCAN_PREVIOUS_TRACK
/*

Todo:
* Option to define/switch "control sets", e.g. one set for Macos and one for
Windows, or for specific apps. Do this by long-pressing the mode select button.
* Encoder wheel acceleration. E.g. in VLC mode, use faster scrub keypress the
longer I continuously rotate the wheel.

Format for controlMode is:

{{"Mode Name"}, {"Scroll Wheel Function Name"},
    {"Left Arrow Function Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"Right Arrow Function Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CCW Scroll Wheel Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CW Scroll Wheel Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"Centre Button Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask}},

Mode Name is required.
Scroll Wheel Function Name optional, pass empty string ("") if you don't want it.
All other action Names are required for that action to work.
If you want these actions to be ignored, you can leave the Name blank.

All keycodes will be sent and released simultaneously.
modeMask is optional; it controls button behavior for long or short keypresses.

**Note** Due to memory limitations you can only have about 5 active control modes
before you run out of memory and the display becomes erratic.
*/

controlMode controlModeList[] = {
    {{"Volume"}, {"Volume"},
     {},
     {},
     {"-", {CONSUMER_HID_TYPE, VOLUME_DOWN_CODE}},
     {"+", {CONSUMER_HID_TYPE, VOLUME_UP_CODE}},
     {"Mute", {CONSUMER_HID_TYPE, VOLUME_MUTE_CODE}}},

    {{"Media"}, {"Volume"},
     {"Prev\nTrack", {CONSUMER_HID_TYPE, TRACK_PREVIOUS_CODE}},
     {"Next\nTrack", {CONSUMER_HID_TYPE, TRACK_NEXT_CODE}},
     {"-", {CONSUMER_HID_TYPE, VOLUME_DOWN_CODE}},
     {"+", {CONSUMER_HID_TYPE, VOLUME_UP_CODE}},
     {"Play/\nPause", {CONSUMER_HID_TYPE, PLAY_PAUSE_CODE}}},

    // {{"Media"}, {"Seek"},
    //  {"Prev\nTrack", {CONSUMER_HID_TYPE, TRACK_PREVIOUS_CODE}},
    //  {"Next\nTrack", {CONSUMER_HID_TYPE, TRACK_NEXT_CODE}},
    //  {"<", {CONSUMER_HID_TYPE, TRACK_SCAN_BACKWARD}, LONG_KEY_DOWN_TIME},
    //  {">", {CONSUMER_HID_TYPE, TRACK_SCAN_FORWARD}, LONG_KEY_DOWN_TIME},
    //  {"Play/\nPause", {CONSUMER_HID_TYPE, PLAY_PAUSE_CODE}}},

    {{"VLC"}, {"Scrub"},
      {"Prev\nTrack",
        {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}}
      },
      {"Next\nTrack",
        {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}}
      },
      {"<",
        {
          {KEYBOARD_HID_TYPE, KEY_LEFT_CTRL},
          {KEYBOARD_HID_TYPE, KEY_LEFT_GUI},
          {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}
        }
      },
      {">",
        {
          {KEYBOARD_HID_TYPE, KEY_LEFT_CTRL},
          {KEYBOARD_HID_TYPE, KEY_LEFT_GUI},
          {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}
        }
      },
      {"Play/\nPause",
        {KEYBOARD_HID_TYPE, KEY_SPACE}
      }
    },

    {{"YouTube"}, {"Scrub"},
     {"Seek\nBack", {KEYBOARD_HID_TYPE, KEY_J}},
     {"Seek\nForward", {KEYBOARD_HID_TYPE, KEY_L}},
     {"<", {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}},
     {">", {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}},
     {"Play/\nPause", {KEYBOARD_HID_TYPE, KEY_SPACE}}},

    // {{"Mouse"}, {"Scroll"},
    //  {"Left\nClick", {MOUSE_HID_TYPE, MOUSE_LEFT_CLICK}},
    //  {"Right\nClick", {MOUSE_HID_TYPE, MOUSE_RIGHT_CLICK}},
    //  {"^", {MOUSE_HID_TYPE, MOUSE_SCROLL_NEGATIVE}},
    //  {"v", {MOUSE_HID_TYPE, MOUSE_SCROLL_POSITIVE}},
    //  {"Middle\nClick", {MOUSE_HID_TYPE, MOUSE_MIDDLE_CLICK}}},

    // {{"Navigation"}, {"Arrow"},
    //  {"Prev\nPage", {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_LEFT_BRACE}}},  // CONSUMER_BROWSER_BACK maybe
    //  {"Next\nPage", {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_RIGHT_BRACE}}}, // CONSUMER_BROWSER_FORWARD maybe
    //  {"^", {KEYBOARD_HID_TYPE, KEY_UP_ARROW}},
    //  {"v", {KEYBOARD_HID_TYPE, KEY_DOWN_ARROW}},
    //  {"Enter", {KEYBOARD_HID_TYPE, KEY_ENTER}}},

    {{"System"}, {"Brightness"},
     {"Ext\n-", {KEYBOARD_HID_TYPE, KEY_SCROLL_LOCK}}, // External Display
     {"Ext\n+", {KEYBOARD_HID_TYPE, KEY_PAUSE}},       // External Display
     {"-", {CONSUMER_HID_TYPE, CONSUMER_BRIGHTNESS_DOWN}}, // Internal Display
     {"+", {CONSUMER_HID_TYPE, CONSUMER_BRIGHTNESS_UP}},   // Internal Display
     {""}},
};
const uint8_t numberOfModes = sizeof (controlModeList) / sizeof (controlModeList[0]);

#define DEFAULT_MODE 1 // Mode to use upon startup

uint8_t currentModeIndex = DEFAULT_MODE;

uint8_t previousModeIndex = currentModeIndex;

// a mode that can be quickly toggled between, ususally volume control
uint8_t toggleModeIndex = 0;

// when did the user last perform an action?
unsigned long lastAction = 0;

// If in toggle mode and no action performed, return to previous mode in this many milliseconds
#define TOGGLE_MODE_EXPIRES_IN 10000 // ms

// How long until the screen saver starts?
#define SCREENSAVER_STARTS_IN 300000 // ms
// #define SCREENSAVER_STARTS_IN 3000 // ms

bool screensaverEnabled = false;
const String screensaverText = "scrnsvr";

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

  oledPrintCentered(currentMode().name + F(" Mode"), 0);

  // oled.setCursor(0, 1);
  // oled.print("01234567890123456789012345678901234567890");

  uint8_t middleRow = ((displayHeightInRows / oled.fontRows())/3);

  // middle button
  oledPrintCentered(currentMode().middle.name, middleRow);

  // left button
  oledPrintLeftJustify(currentMode().left.name, middleRow);

  // right button
  oledPrintRightJustify(currentMode().right.name, middleRow);

  // wheel actions
  uint8_t wheelActionRow = (displayHeightInRows - 1) - (oled.fontRows() * 2);
  oledPrintCentered((currentMode().wheelCW.name + F(" ") + currentMode().wheelName + F(" ") + currentMode().wheelCCW.name), wheelActionRow);

  // mode quick-toggle
  uint8_t quickToggleRow = (displayHeightInRows) - oled.fontRows();
  if (previousModeIndex == currentModeIndex) {
    if (currentModeIndex != toggleModeIndex) {
      oledPrintCentered(toggleMode().name + F(" ->"), quickToggleRow);
    } else {
      oledPrintCentered(F("--"), quickToggleRow);
    }
  } else {
    if (previousModeIndex != toggleModeIndex) {
      oledPrintCentered("<- " + previousMode().name, quickToggleRow);
    } else {
      oledPrintCentered(toggleMode().name + F(" ->"), quickToggleRow);
    }
  }
}

bool inToggleMode() {
  return ((previousModeIndex != currentModeIndex) && (previousModeIndex != toggleModeIndex));
}

void updateLastAction() {
  lastAction = millis();
  if (screensaverEnabled) {
    screensaverEnabled = false;
    updateDisplay();
  }
}

void setup() {
  #ifdef ENABLE_DEBUGGING
    Serial.begin(9600);
  #endif

  mcuSetup();

  displaySetup();

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

#define OUTPUT_EVERY 5000
unsigned long nextOutput = OUTPUT_EVERY;

boolean keyboardPressed = false; // is Keyboard in currently-pressed state?
boolean consumerPressed = false; // is Consumer in currently-pressed state?

// Send action but don't release the keys
void sendAction(controlAction actionToSend)
{
  setIndicatorLed(1);
  updateLastAction();

  debugf("Sending key action '");
  debug(actionToSend.name);
  debugfln("'");

  for (uint8_t i = 0; i < MAX_KEYS_PER_ACTION; i++) {
    if (sizeof(actionToSend.keys[i]) > 0) {

      debuglnfmt(actionToSend.keys[i].keyCode, HEX);
      if (actionToSend.keys[i].hidType == CONSUMER_HID_TYPE) {
        debugf("CONSUMER_HID_TYPE ");
        Consumer.press(actionToSend.keys[i].keyCode);
        consumerPressed = true;
      } else if (actionToSend.keys[i].hidType == MOUSE_HID_TYPE) {
        debugf("MOUSE_HID_TYPE ");
        // if (bitRead(actionToSend.modeMask, 7)) {
          if (bitRead(actionToSend.modeMask, 6)) {
            debugfln("scrolling down");
            Mouse.move(0, 0, MOUSE_SCROLL_AMOUNT);
          } else if (bitRead(actionToSend.modeMask, 5)) {
            debugfln("scrolling up");
            Mouse.move(0, 0, -MOUSE_SCROLL_AMOUNT);
          } else if (bitRead(actionToSend.modeMask, 4)) {
            debugfln("left click");
            Mouse.click(MOUSE_LEFT);
          } else if (bitRead(actionToSend.modeMask, 3)) {
            debugfln("right click");
            Mouse.click(MOUSE_RIGHT);
          } else if (bitRead(actionToSend.modeMask, 2)) {
            debugfln("middle click");
            Mouse.click(MOUSE_MIDDLE);
          }
        // }
      } else {
        debugf("KEYBOARD_HID_TYPE ");
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
      oled.setCursor(random(0, oled.displayWidth()-oled.fieldWidth(screensaverText.length())), random(0, displayHeightInRows));
      oled.print(screensaverText);
    }
  }

  if (upButton.isPressed() && middleButton.wasPressed()) {
    if (currentFont++ >= nFont-1)
      currentFont = 0;

    setFont(currentFont);
    oled.clear();
    oled.print(F("Font: "));
    oled.print(fontName[currentFont]);
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
