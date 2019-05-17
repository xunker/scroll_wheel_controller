// https://github.com/NicoHood/HID/blob/master/examples/Consumer/Consumer.ino
// https://github.com/NicoHood/HID/blob/cc946b9dd7b73b61293a4aa74444de3a5a871bb0/src/KeyboardLayouts/ImprovedKeylayoutsUS.h
// https://github.com/NicoHood/HID/blob/d4938ddcff7970bc1d32a040a08afeac4915e4a9/src/HID-APIs/ConsumerAPI.h
#include "HID-Project.h"

#include <JC_Button.h> // https://github.com/JChristensen/JC_Button

#include <Encoder.h> // https://www.pjrc.com/teensy/td_libs_Encoder.html

#define MIDDLE_PIN 7
#define UP_PIN 8
#define DOWN_PIN 4
#define LEFT_PIN 5
#define RIGHT_PIN 6
#define ENC_PIN_A 2 // need to be interrupt pin
#define ENC_PIN_B 3 // need to be interrupt pin

#define ENCODER_STEPS 2 // number of increments per encoder detent

Button middleButton(MIDDLE_PIN);
Button upButton(UP_PIN);
Button downButton(DOWN_PIN);
Button leftButton(LEFT_PIN);
Button rightButton(RIGHT_PIN);

Encoder encoder(ENC_PIN_A, ENC_PIN_B);

// maybe? https://forum.arduino.cc/index.php?topic=157618.0
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

combine modeMasks using |
*/
#define REGULAR_KEY_DOWN_TIME 0b00000001
#define LONG_KEY_DOWN_TIME 0b00000001

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

#define NUMBER_OF_MODES 4
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

    {{"Navigation"},
     {"Back", KEY_LEFT_GUI, KEY_LEFT_BRACE, NULL, NULL},     // CONSUMER_BROWSER_BACK maybe
     {"Forward", KEY_LEFT_GUI, KEY_RIGHT_BRACE, NULL, NULL}, // CONSUMER_BROWSER_FORWARD maybe
     {"Up", KEY_UP_ARROW, NULL, NULL, NULL},
     {"Down", KEY_DOWN_ARROW, NULL, NULL, NULL},
     {"Select", KEY_ENTER, NULL, NULL, NULL}}};

// controlMode currentMode = vlc;
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

void
setup()
{
  Serial.begin(9600);
  delay(5000);

  pinMode(MIDDLE_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
  pinMode(ENC_PIN_A, INPUT_PULLUP);
  pinMode(ENC_PIN_B, INPUT_PULLUP);
  // Consumer.begin();

  Keyboard.begin();
}

// bool pressed = false;

#define OUTPUT_EVERY 5000
unsigned long nextOutput = OUTPUT_EVERY;

void sendAction(controlAction actionToSend)
{

  Serial.print("Key action is '");
  Serial.print(actionToSend.name);
  Serial.println("'");

  for (uint8_t i = 0; i < MAX_KEYS_PER_ACTION; i++) {
    if (actionToSend.keys[i]) {
      Serial.println(actionToSend.keys[i], HEX);
      Keyboard.press(actionToSend.keys[i]);
    }
  }

  if (actionToSend.consumerKey) {
    Serial.println(actionToSend.consumerKey, HEX);
    Keyboard.press(actionToSend.consumerKey);
  }

  if (actionToSend.modeMask & 0b00000001) {
    Serial.println("Using KEY_DOWN_TIME_LONG");
    delay(KEY_DOWN_TIME_LONG);
  } else {
    delay(KEY_DOWN_TIME_REGULAR);
  }

  Keyboard.releaseAll();
}

void changeModeMessage() {
  Serial.print("Changing control mode to ");
  Serial.print(currentModeIndex);
  Serial.print(" '");
  Serial.print(currentMode().name);
  Serial.println("'");
}

void loop() {
  middleButton.read();
  upButton.read();
  downButton.read();
  leftButton.read();
  rightButton.read();

  long encoderPos = encoder.read();

  if (nextOutput < millis()) {
    nextOutput = millis() + OUTPUT_EVERY;

    Serial.print(millis());
    Serial.print(" Current Control Mode is '");
    Serial.print(currentMode().name);
    Serial.println("'");

  }

  if (middleButton.wasPressed()) {
    sendAction(currentMode().middle);
  } else if (upButton.wasPressed()) {
    currentModeIndex++;
    if (currentModeIndex >= NUMBER_OF_MODES) {
      currentModeIndex = 0;
    }
    previousModeIndex = currentModeIndex;

    changeModeMessage();
  }
  else if (downButton.wasPressed()) {
    if (currentModeIndex == toggleModeIndex) {
      if (previousModeIndex != toggleModeIndex) {
        Serial.print("Returning to previous mode");
        currentModeIndex = previousModeIndex;
        changeModeMessage();
      } else {
        Serial.println("Toggle mode and previous mode are the same");
      }
    } else {
      Serial.print("Temporary toggle to '");
      Serial.print(toggleMode().name);
      Serial.println("'");

      currentModeIndex = toggleModeIndex;
    }
  }
  else if (leftButton.wasPressed()) {
    sendAction(currentMode().left);
  }
  else if (rightButton.wasPressed()) {
    sendAction(currentMode().right);
  }

  if (encoderPos > 0) {
    sendAction(currentMode().wheelCCW);
  } else if (encoderPos < 0) {
    sendAction(currentMode().wheelCW);
  }
  if (encoderPos != 0) {
    encoder.write(0);
  }
}
