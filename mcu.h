// #define ARDUINO_MICRO
#define SPARKFUN_PRO_MICRO // https://learn.sparkfun.com/tutorials/pro-micro--fio-v3-hookup-guide

#ifdef ARDUINO_MICRO
  // SDA is 2, SCL is 3
  #define MIDDLE_PIN 7
  #define UP_PIN 10
  #define DOWN_PIN 4
  #define LEFT_PIN 5
  #define RIGHT_PIN 6
  #define ENC_PIN_A 8
  #define ENC_PIN_B 9

  void mcuSetup() {
    pinMode(LED_BUILTIN, OUTPUT);
  }

  void setIndicatorLed(bool state) {
    if (state) {
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }
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

  void mcuSetup() {
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    TXLED0;
  }

  void setIndicatorLed(bool state) {
    if (state) {
      digitalWrite(LED_BUILTIN, LOW);
      // TXLED1;
    } else {
      digitalWrite(LED_BUILTIN, HIGH);
      // TXLED0;
    }
  }

#endif
