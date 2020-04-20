#ifndef CONTROL_MODE_STRUCTS_H
#define CONTROL_MODE_STRUCTS_H

/* The struct objects used in control-mode definitions */

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
  controlAction middle;
  controlAction wheelCW;
  controlAction wheelCCW;
};


#endif
