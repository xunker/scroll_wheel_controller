#ifndef CONTROL_MODES_H
#define CONTROL_MODES_H

/*
Format for controlMode is:

{{"Mode Name"}, {"Scroll Wheel Function Name"},
    {"Left Arrow Function Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"Right Arrow Function Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CCW Scroll Wheel Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"CW Scroll Wheel Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask},
    {"Centre Button Action Name", {{KEY_TYPE, KEY_CODE}, ..., {KEY_TYPE, KEY_CODE}}, modeMask}},

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
  * 0b00000001 - long keypress time (1,100ms)

**Note** Due to memory limitations you can only have about 5 active control modes
before you run out of memory and the display becomes erratic.
*/

const controlMode controlModeList[] = {
    {{"Volume"}, {"Volume"},
     {},
     {},
     {"-", {CONSUMER_HID_TYPE, MEDIA_VOLUME_DOWN}},
     {"+", {CONSUMER_HID_TYPE, MEDIA_VOLUME_UP}},
     {"Mute", {CONSUMER_HID_TYPE, MEDIA_VOLUME_MUTE}}},

    {{"Media"}, {"Volume"},
     {"Prev\n<<", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_PREVIOUS_TRACK}},
     {"Next\n>>", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_NEXT_TRACK}},
     {"-", {CONSUMER_HID_TYPE, MEDIA_VOLUME_DOWN}},
     {"+", {CONSUMER_HID_TYPE, MEDIA_VOLUME_UP}},
     {"Play/\nPause", {CONSUMER_HID_TYPE, MEDIA_PLAY_PAUSE}}},

    // {{"Media"}, {"Seek"},
    //  {"Prev\nTrack", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_PREVIOUS_TRACK}},
    //  {"Next\nTrack", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_NEXT_TRACK}},
    //  {"<", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_PREVIOUS_TRACK}, LONG_KEY_DOWN_TIME},
    //  {">", {CONSUMER_HID_TYPE, HID_CONSUMER_SCAN_NEXT_TRACK}, LONG_KEY_DOWN_TIME},
    //  {"Play/\nPause", {CONSUMER_HID_TYPE, MEDIA_PLAY_PAUSE}}},

    {{"VLC"}, {"Scrub"},
      {"Prev\n<<",
        {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}}
      },
      {"Next\n>>",
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
     {"Seek\n<<", {KEYBOARD_HID_TYPE, KEY_J}},
     {"Seek\n>>", {KEYBOARD_HID_TYPE, KEY_L}},
     {"<", {KEYBOARD_HID_TYPE, KEY_LEFT_ARROW}},
     {">", {KEYBOARD_HID_TYPE, KEY_RIGHT_ARROW}},
     {"Play/\nPause", {KEYBOARD_HID_TYPE, KEY_SPACE}}},

    {{"Mouse"}, {"Scroll"},
     {"Left\nClick", {MOUSE_HID_TYPE, MOUSE_LEFT_CLICK}},
     {"Right\nClick", {MOUSE_HID_TYPE, MOUSE_RIGHT_CLICK}},
     {"^", {MOUSE_HID_TYPE, MOUSE_SCROLL_NEGATIVE}},
     {"v", {MOUSE_HID_TYPE, MOUSE_SCROLL_POSITIVE}},
     {"Middle\nClick", {MOUSE_HID_TYPE, MOUSE_MIDDLE_CLICK}}},

    {{"Navigation"}, {"Arrow"},
     {"Prev\nPage", {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_LEFT_BRACE}}},  // CONSUMER_BROWSER_BACK maybe
     {"Next\nPage", {{KEYBOARD_HID_TYPE, KEY_LEFT_GUI}, {KEYBOARD_HID_TYPE, KEY_RIGHT_BRACE}}}, // CONSUMER_BROWSER_FORWARD maybe
     {"^", {KEYBOARD_HID_TYPE, KEY_UP_ARROW}},
     {"v", {KEYBOARD_HID_TYPE, KEY_DOWN_ARROW}},
     {"Enter", {KEYBOARD_HID_TYPE, KEY_ENTER}}},

    {{"System"}, {"Brightness"},
     {"Ext\n-", {KEYBOARD_HID_TYPE, KEY_SCROLL_LOCK}}, // External Display
     {"Ext\n+", {KEYBOARD_HID_TYPE, KEY_PAUSE}},       // External Display
     {"-", {CONSUMER_HID_TYPE, CONSUMER_BRIGHTNESS_DOWN}}, // Internal Display
     {"+", {CONSUMER_HID_TYPE, CONSUMER_BRIGHTNESS_UP}},   // Internal Display
     {}},
};

/* Index of mode to use upon startup, starting from 0 (zero) */
#define DEFAULT_MODE 1

/* How many lines to scroll for mouse wheel events */
#define MOUSE_SCROLL_AMOUNT 5

#endif
