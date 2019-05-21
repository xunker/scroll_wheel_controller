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
