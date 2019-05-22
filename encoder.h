// Be sure to enable half-step mode
#include <Rotary.h>  // https://github.com/brianlow/Rotary

Rotary encoder = Rotary(ENC_PIN_A, ENC_PIN_B);

// to be called from inside main setup()
void encoderSetup() {
  pinMode(ENC_PIN_A, INPUT_PULLUP);
  pinMode(ENC_PIN_B, INPUT_PULLUP);

  // https: //github.com/brianlow/Rotary/blob/master/examples/InterruptProMicro/InterruptProMicro.ino
  encoder.begin();
  PCICR |= (1 << PCIE0);
  PCMSK0 |= (1 << PCINT4) | (1 << PCINT5);
  sei();
}

// https: //github.com/brianlow/Rotary/blob/master/examples/InterruptProMicro/InterruptProMicro.ino
volatile uint8_t encoderTurnedCW = 0;
volatile uint8_t encoderTurnedCCW = 0;
ISR(PCINT0_vect) {
  unsigned char result = encoder.process();
  if (result == DIR_NONE) {
    // do nothing
  }
  else if (result == DIR_CW) {
    encoderTurnedCW++;
  }
  else if (result == DIR_CCW) {
    encoderTurnedCCW++;
  }
}
