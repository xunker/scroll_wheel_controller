#include <JC_Button.h> // https://github.com/JChristensen/JC_Button

Button middleButton(MIDDLE_PIN);
Button upButton(UP_PIN);
Button downButton(DOWN_PIN);
Button leftButton(LEFT_PIN);
Button rightButton(RIGHT_PIN);

// to be called from inside main setup()
void buttonsSetup() {
  pinMode(MIDDLE_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(LEFT_PIN, INPUT_PULLUP);
  pinMode(RIGHT_PIN, INPUT_PULLUP);
}

void readButtons() {
  middleButton.read();
  upButton.read();
  downButton.read();
  leftButton.read();
  rightButton.read();
}