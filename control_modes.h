#ifndef CONTROL_MODES_H
#define CONTROL_MODES_H

#include "control_mode_structs.h"

/*
Format for controlMode is:

{{"Mode Name"}, {"Scroll Wheel Function Name"},
    {"Left Arrow Function Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"Right Arrow Function Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"Centre Button Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CCW Scroll Wheel Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CW Scroll Wheel Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CCW Scroll Wheel Accelerated Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CW Scroll Wheel Accelerated Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask}},

Mode Name is required. This is what is displayed at the top of the screen.

"* Function Name" is is required for each function, even if it is an empty
string. If the button/function is ignored, a `{}` can be passed instead.

An array of up to MAX_KEYS_PER_ACTION keys (default 3) can be passed in an array
for each function. The `KEY_TYPE` can be one of:

  * KEYBOARD_HID_TYPE - for standard keyboard keys
  * CONSUMER_HID_TYPE - for "consumer" keys like volume and media control
  * MOUSE_HID_TYPE    - for mouse clicks/wheel scrolls

All keycodes will be sent and released simultaneously.

modeMask is optional; it controls button behavior for long or short keypresses.
It an be one of:
  * 0b00000000 - default (short) keypress time (10ms)
  * 0b00000001 - long keypress time (700ms)

**Note** Due to memory limitations you can only have about 5-7 active control modes
before you run out of memory and the display becomes erratic.

**REMEMBER**: Any text label cannot be longer than MAX_LABEL_LENGTH (default 12 characters)
*/

const controlMode controlModeList[] = {
    {{"Volume"}, {"Volume"},
     {},
     {},
     {"Mute", {CONSUMER_HID_TYPE, MEDIA_VOLUME_MUTE}},
     {"-", {CONSUMER_HID_TYPE, MEDIA_VOLUME_DOWN}},
     {"+", {CONSUMER_HID_TYPE, MEDIA_VOLUME_UP}}},

    {{"Media"}, {"Volume"},
     {"Prev\n<<", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_PREVIOUS_TRACK}},
     {"Next\n>>", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_NEXT_TRACK}},
     {"Play\nPause", {CONSUMER_HID_TYPE, MEDIA_PLAY_PAUSE}},
     {"-", {CONSUMER_HID_TYPE, MEDIA_VOLUME_DOWN}},
     {"+", {CONSUMER_HID_TYPE, MEDIA_VOLUME_UP}}},

    // {{"Media"}, {"Seek"},
    //  {"Prev\nTrack", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_PREVIOUS_TRACK}},
    //  {"Next\nTrack", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_NEXT_TRACK}},
    //  {"Play\nPause", {CONSUMER_HID_TYPE, MEDIA_PLAY_PAUSE}},
    //  {"<", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_PREVIOUS_TRACK}, LONG_KEY_DOWN_TIME},
    //  {">", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_NEXT_TRACK}, LONG_KEY_DOWN_TIME}},

    {{"VLC"}, {"Scrub"},
      {"Prev\n<<",
        {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}}
      },
      {"Next\n>>",
        {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}}
      },
      {"Play\nPause",
        {KEYBOARD_HID_TYPE, KEY_SPACE}
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
      {"<<",
        {
          {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}
        }
      },
      {">>",
        {
          {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}
        }
      }
    },

    {{"YouTube"}, {"Scrub"},
     {"Seek\n<<", {KEYBOARD_HID_TYPE, KEY_J}},
     {"Seek\n>>", {KEYBOARD_HID_TYPE, KEY_L}},
     {"Play\nPause", {KEYBOARD_HID_TYPE, KEY_SPACE}},
     {"<", {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}},
     {">", {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}}},

    {{"Mouse"}, {"Scroll"},
     {"Left\nBtn", {MOUSE_HID_TYPE, MOUSE_LEFT_CLICK}},
     {"Right\nBtn", {MOUSE_HID_TYPE, MOUSE_RIGHT_CLICK}},
     {"Mid\nBtn", {MOUSE_HID_TYPE, MOUSE_MIDDLE_CLICK}},
     {"^", {MOUSE_HID_TYPE, MOUSE_SCROLL_NEGATIVE}},
     {"_", {MOUSE_HID_TYPE, MOUSE_SCROLL_POSITIVE}}},

    // {{"Navigation"}, {"Page"},
    //  {"Prev\nPage", {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_LEFT_BRACE}}},  // CONSUMER_BROWSER_BACK maybe
    //  {"Next\nPage", {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_RIGHT_BRACE}}}, // CONSUMER_BROWSER_FORWARD maybe
    //  {"Enter", {KEYBOARD_HID_TYPE, KEY_ENTER}},
    //  {"^", {KEYBOARD_HID_TYPE, KEY_PAGE_UP}},
    //  {"v", {KEYBOARD_HID_TYPE, KEY_PAGE_DOWN}}},

    {{"System"}, {"Bright"},
     {"Ext\n-", {KEYBOARD_HID_TYPE, KEY_SCROLL_LOCK}}, // External Display
     {"Ext\n+", {KEYBOARD_HID_TYPE, KEY_PAUSE}},       // External Display
     {},
     {"-", {CONSUMER_HID_TYPE, CONSUMER_BRIGHTNESS_DOWN}}, // Internal Display
     {"+", {CONSUMER_HID_TYPE, CONSUMER_BRIGHTNESS_UP}}},   // Internal Display
};

/* Index of mode to use upon startup, starting from 0 (zero) */
#define DEFAULT_MODE 1

/* How many lines to scroll for mouse wheel events */
#define MOUSE_SCROLL_AMOUNT 5

#endif
