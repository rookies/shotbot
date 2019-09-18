#include <AccelStepper.h>
#include "Config.hh"

const long stepsPerPosition = (positionMax - positionMin) / (positionsNum - 1);
uint8_t positionIdxCurrent = 0;

AccelStepper stepper(AccelStepper::DRIVER, pinStep, pinDirection);

void home() {
  /* Run backwards until the endstop switch is pressed: */
  stepper.setSpeed(-homeStepsPerSecond);
  while (digitalRead(pinEndstop) == HIGH) {
    stepper.runSpeed();
  }
  stepper.stop();
  /* Zero the position: */
  stepper.setCurrentPosition(0);
  /* Play a short melody to signal that we're ready: */
  tone(pinStep, 8372); // C
  delay(500);
  tone(pinStep, 10548); // E
  delay(500);
  tone(pinStep, 12544); // G
  delay(500);
  noTone(pinStep);
  delay(1000);
  /* Go to minimal position: */
  stepper.runToNewPosition(positionMin);
}

void setup() {
  /* Setup stepper motor: */
  stepper.setEnablePin(pinEnable);
  stepper.setPinsInverted(false, false, true);
  stepper.enableOutputs();
  stepper.setMaxSpeed(maxStepsPerSecond);
  stepper.setAcceleration(acceleration);
  /* Setup endstop switch: */
  pinMode(pinEndstop, INPUT);
  /* Go to home position: */
  home();
}

void loop() {
  /* If endstop switch is pressed, stop instantly: */
  if (digitalRead(pinEndstop) == LOW) {
    stepper.stop();
    return;
    /* TODO: Enter some kind of emergency mode? */
  }
  /* Move to the next position if we arrived at the current one: */
  if (stepper.distanceToGo() == 0) {
    positionIdxCurrent++;
    if (positionIdxCurrent == positionsNum) {
      positionIdxCurrent = 0;
    }
    stepper.moveTo(positionMin + positionIdxCurrent * stepsPerPosition);
  }
  /* Run the motor: */
  stepper.run();
}
