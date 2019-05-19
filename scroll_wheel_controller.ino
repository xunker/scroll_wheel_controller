// https://github.com/NicoHood/HID/blob/master/examples/Consumer/Consumer.ino
// https://github.com/NicoHood/HID/blob/cc946b9dd7b73b61293a4aa74444de3a5a871bb0/src/KeyboardLayouts/ImprovedKeylayoutsUS.h
// https://github.com/NicoHood/HID/blob/d4938ddcff7970bc1d32a040a08afeac4915e4a9/src/HID-APIs/ConsumerAPI.h
#include "HID-Project.h"

#include <JC_Button.h> // https://github.com/JChristensen/JC_Button

// Be sure to enable half-step mode
#include <Rotary.h>  // https://github.com/brianlow/Rotary

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

// #define ARDUINO_MICRO
#define SPARKFUN_PRO_MICRO // https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide

#define LED_BUILTIN_ON HIGH
#define LED_BUILTIN_OFF LOW

#ifdef ARDUINO_MICRO
  // SDA is 2, SCL is 3
  #define MIDDLE_PIN 7
  #define UP_PIN 10
  #define DOWN_PIN 4
  #define LEFT_PIN 5
  #define RIGHT_PIN 6
  #define ENC_PIN_A 8
  #define ENC_PIN_B 9
#endif

#ifdef SPARKFUN_PRO_MICRO
  // SDA is 2, SCL is 3
  #define MIDDLE_PIN 7
  #define UP_PIN 10
  #define DOWN_PIN 4
  #define LEFT_PIN 5
  #define RIGHT_PIN 6
  #define ENC_PIN_A 8
  #define ENC_PIN_B 9

  #define LED_BUILTIN 17 // rx led on pro micro, has no led on 13
  #define LED_BUILTIN_ON LOW
  #define LED_BUILTIN_OFF HIGH

#endif

#define MOUSE_SCROLL_AMOUNT 5

Button middleButton(MIDDLE_PIN);
Button upButton(UP_PIN);
Button downButton(DOWN_PIN);
Button leftButton(LEFT_PIN);
Button rightButton(RIGHT_PIN);

Rotary encoder = Rotary(ENC_PIN_A, ENC_PIN_B);

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


combine modeMasks using |
*/

#define LONG_KEY_DOWN_TIME 0b00000001

#define MOUSE_EVENT 0b10000000
#define MOUSE_SCROLL_POSITIVE MOUSE_EVENT | 0b01000000
#define MOUSE_SCROLL_NEGATIVE MOUSE_EVENT | 0b00100000

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
controlMode controlModeList[NUMBER_OF_MODES] = {
    {{"Volume"},
     {"", NULL, NULL, NULL, NULL},
     {"", NULL, NULL, NULL, NULL},
     {"Vol -", NULL, NULL, NULL, MEDIA_VOLUME_DOWN},
     {"Vol +", NULL, NULL, NULL, MEDIA_VOLUME_UP},
     {"Mute", NULL, NULL, NULL, MEDIA_VOLUME_MUTE}},

    {{"Media"},
     {"Previous", NULL, NULL, NULL, MEDIA_PREVIOUS},
     {"Next", NULL, NULL, NULL, MEDIA_NEXT},
     {"Seek <", NULL, NULL, NULL, MEDIA_REWIND, LONG_KEY_DOWN_TIME},
     {"Seek >", NULL, NULL, NULL, MEDIA_FAST_FORWARD, LONG_KEY_DOWN_TIME},
     {"Play/Pause", NULL, NULL, NULL, MEDIA_PLAY_PAUSE}},

    {{"VLC"},
     {"Previous", KEY_LEFT_GUI, KEY_LEFT_ARROW, NULL, NULL},
     {"Next", KEY_LEFT_GUI, KEY_RIGHT_ARROW, NULL, NULL},
     {"Scrub <", KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_LEFT_ARROW, NULL},
     {"Scrub >", KEY_LEFT_CTRL, KEY_LEFT_GUI, KEY_RIGHT_ARROW, NULL},
     {"Play/Pause", KEY_SPACE, NULL, NULL, NULL}},

    {{"Mouse"},
     {"", NULL, NULL, NULL, NULL},
     {"", NULL, NULL, NULL, NULL},
     {"Up", NULL, NULL, NULL, NULL, MOUSE_SCROLL_NEGATIVE},
     {"Down", NULL, NULL, NULL, NULL, MOUSE_SCROLL_POSITIVE},
     {"", NULL, NULL, NULL, NULL}},

    {{"Navigation"},
     {"Back", KEY_LEFT_GUI, KEY_LEFT_BRACE, NULL, NULL},     // CONSUMER_BROWSER_BACK maybe
     {"Forward", KEY_LEFT_GUI, KEY_RIGHT_BRACE, NULL, NULL}, // CONSUMER_BROWSER_FORWARD maybe
     {"Up", KEY_UP_ARROW, NULL, NULL, NULL},
     {"Down", KEY_DOWN_ARROW, NULL, NULL, NULL},
     {"Select", KEY_ENTER, NULL, NULL, NULL}}};

uint8_t currentModeIndex = 0;

uint8_t previousModeIndex = currentModeIndex;

// a mode that can be quickly toggled between, ususally volume control
uint8_t toggleModeIndex = 0;

controlMode currentMode() {
  return controlModeList[currentModeIndex];
}

controlMode toggleMode() {
  return controlModeList[toggleModeIndex];
}

void setup() {
  #ifdef USE_SERIAL
    Serial.begin(9600);
  #endif
  delay(5000);

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LED_BUILTIN_OFF);

  pinMode(MIDDLE_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(ENC_PIN_A, INPUT_PULLUP);
  pinMode(ENC_PIN_B, INPUT_PULLUP);

  // Consumer.begin();

  Keyboard.begin();
  Mouse.begin();

  // https: //github.com/brianlow/Rotary/blob/master/examples/InterruptProMicro/InterruptProMicro.ino
  encoder.begin();
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT4) | (1 << PCINT5);
  sei();
}

// https: //github.com/brianlow/Rotary/blob/master/examples/InterruptProMicro/InterruptProMicro.ino
bool encoderTurnedCW = false;
bool encoderTurnedCCW = false;
ISR(PCINT0_vect) {
  unsigned char result = encoder.process();
  if (result == DIR_NONE) {
    // do nothing
  }
  else if (result == DIR_CW) {
    encoderTurnedCW = true;
  }
  else if (result == DIR_CCW) {
    encoderTurnedCCW = true;
  }
}

#define OUTPUT_EVERY 5000
unsigned long nextOutput = OUTPUT_EVERY;

// Send action but don't release the keys
void sendAction(controlAction actionToSend)
{
  digitalWrite(LED_BUILTIN, LED_BUILTIN_ON);

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
  middleButton.read();
  upButton.read();
  downButton.read();
  leftButton.read();
  rightButton.read();

  if (nextOutput < millis()) {
    nextOutput = millis() + OUTPUT_EVERY;

    debug(millis());
    debug(" Current Control Mode is '");
    debug(currentMode().name);
    debugln("'");

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
    currentModeIndex++;
    if (currentModeIndex >= NUMBER_OF_MODES) {
      currentModeIndex = 0;
    }
    previousModeIndex = currentModeIndex;

    changeModeMessage();
  } else if (downButton.wasPressed()) {
    if (currentModeIndex == toggleModeIndex) {
      if (previousModeIndex != toggleModeIndex) {
        debugln("Returning to previous mode");
        currentModeIndex = previousModeIndex;
        changeModeMessage();
      } else {
        debugln("Toggle mode and previous mode are the same");
      }
    } else {
      debug("Temporary toggle to '");
      debug(toggleMode().name);
      debugln("'");

      currentModeIndex = toggleModeIndex;
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
}
