#ifndef CONFIG_H
#define CONFIG_H

#include "mcu.h" // Config options for specific microcontrollers, such as which pins to use

/* Enable Serial debugging */
// #define ENABLE_DEBUGGING

/* If debugging is enabled, this is the baud rate */
#define DEBUG_BAUD 57600

/* Define how long the virtual keys should be pressed for either a "regular"
   keypress or a "long" keypress. Milliseconds. */
#define KEY_DOWN_TIME_REGULAR 10
#define KEY_DOWN_TIME_LONG 700

/* If in toggle mode and no action performed, return to previous mode in this
   many milliseconds */
#define TOGGLE_MODE_EXPIRES_IN 10000

/* How long until the screen saver starts? Milliseconds */
#define SCREENSAVER_STARTS_IN 300000
// #define SCREENSAVER_STARTS_IN 3000

/* Text displayed on screen when in screensaver mode */
#define SCREENSAVER_TEXT "scrnsvr"

/* If ENABLE_DEBUGGING is on, this is how often the current state is reported
   on the Serial port. If screensaver mode is enabled, this is also how often
   the screensaver text is moved. Milliseconds */
#define OUTPUT_EVERY 5000

/* max number of key/mouse actions that can be attached to a single action */
#define MAX_KEYS_PER_ACTION 3

/* Maximum length of any text label field */
#define MAX_LABEL_LENGTH 11

/* Wheel acceleration: trigger acceleration when CLICK_ACCEL_TRIGGER or more
   wheel clicks happen in CLICK_ACCEL_EVERY milliseconds (if the current
   control mode has a wheel acceleration mode configured). */
#define CLICK_ACCEL_EVERY 250 // milliseconds
// trigger accel when wheel clicked more than this in CLICK_ACCEL_EVERY time
#define CLICK_ACCEL_TRIGGER 5 


#endif
