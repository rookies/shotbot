#include <AccelStepper.h>
#include <Adafruit_NeoPixel.h>
#include "Config.hh"
#include "TaskScheduler.hh"
#include "CommandReader.hh"
#include "SelectorKnob.hh"
#include "DebouncedSwitch.hh"
#include "State.hh"

const long stepsPerPosition = (positionMax - positionMin) / (positionsNum - 1);
uint8_t targetPosition;
uint8_t currentlyActivePump = -1;
bool targetPositionReached = true;
bool endstopErrorPrinted = false;
bool homed = false;


void parseCommand(char *);


/* Inputs: */
SelectorKnob countSelector(pinCountSelector, positionsNum, selectorMaxValue,
                           selectorHysteresis);
SelectorKnob pumpSelector(pinPumpSelector, pumpsNum, selectorMaxValue,
                          selectorHysteresis);
DebouncedSwitch startButton(pinStartButton, true, 100);


/* Outputs: */
AccelStepper stepper(AccelStepper::DRIVER, pinStep, pinDirection);
Adafruit_NeoPixel countLEDs(countLEDsNum, pinCountLEDs, NEO_KHZ800 + NEO_GRB);
Adafruit_NeoPixel pumpLEDs(pumpsNum, pinPumpLEDs, NEO_KHZ800 + NEO_GRB);


/* Utilities: */
TaskScheduler<1> taskScheduler;
CommandReader<commandLengthMax> commandReader(parseCommand);


/* Task IDs: */
const size_t taskId_switchOffPump = 0;


/* HW Control: */
void pumpOn(uint8_t i, long duration) {
  if (i >= pumpsNum) return;

  if (duration <= 0 || duration > pumpMaxTime) {
    duration = pumpMaxTime;
  }

  digitalWrite(pinPumps[i], HIGH);
  state_set_onoff(F("PUMP"), i, true, duration);

  /* Schedule switching off the pump after pumpMaxTime: */
  currentlyActivePump = i;
  taskScheduler.scheduleTask(taskId_switchOffPump, duration);
}
void pumpOff() {
  if (currentlyActivePump >= pumpsNum) return;

  digitalWrite(pinPumps[currentlyActivePump], LOW);
  state_set_onoff(F("PUMP"), currentlyActivePump, false);

  /* Unschedule switching off the pump after pumpMaxTime: */
  taskScheduler.unscheduleTask(taskId_switchOffPump);
}
void stepperOn() {
  stepper.enableOutputs();
  state_set_onoff(F("STEPPER"), true);
}
void stepperOff() {
  stepper.disableOutputs();
  state_set_onoff(F("STEPPER"), false);

  /* Remember that we're not homed anymore: */
  homed = false;
  state_set(F("HOMED"), false);
}


void setup() {
  /* Setup serial port: */
  Serial.begin(serialBaudrate);

  /* Setup stepper motor: */
  stepper.setEnablePin(pinEnable);
  stepper.setPinsInverted(false, false, true);
  stepper.setMaxSpeed(maxStepsPerSecond);
  stepper.setAcceleration(acceleration);
  stepperOn();

  /* Setup endstop switch: */
  pinMode(pinEndstop, INPUT);

  /* Setup pumps: */
  for (uint8_t i=0; i < pumpsNum; i++) {
    pinMode(pinPumps[i], OUTPUT);
    currentlyActivePump = i;
    pumpOff();
  }
  currentlyActivePump = -1;

  /* Setup LEDs: */
  countLEDs.begin();
  pumpLEDs.begin();

  /* Setup tasks: */
  taskScheduler.setTask(taskId_switchOffPump, pumpOff);

  /* We're not homed yet: */
  homed = false;
  state_set(F("HOMED"), false);
  state_set(F("TARGET"), -1);
  state_set(F("POSITION"), -1);
  state_set(F("BLOCKED"), false);

  /* Report that we're ready: */
  state_set(F("READY"), true);
  state_set(F("POSITIONS"), positionsNum);
  state_set(F("PUMPS"), pumpsNum);
}


void cmd_home() {
  state_set(F("BLOCKED"), true);

  /* Run backwards until the endstop switch is pressed: */
  stepper.setSpeed(-homeStepsPerSecond);
  while (digitalRead(pinEndstop) == HIGH) {
    stepper.runSpeed();
  }
  stepper.stop();

  /* Zero the position: */
  stepper.setCurrentPosition(0);

  /* Go to minimal position: */
  stepper.runToNewPosition(positionMin);

  /* Remember that we homed: */
  homed = true;
  state_set(F("HOMED"), true);

  /* Remember the current position: */
  targetPosition = 0;
  state_set(F("POSITION"), 0);

  /* Suppress POSREACHED message: */
  targetPositionReached = true;

  state_set(F("BLOCKED"), false);
}


void cmd_goto(char *command) {
  /* Check if we're homed: */
  if (!homed) {
    Serial.println(F("CMD ERROR NOTHOMED"));
    return;
  }

  /* Extract position: */
  int position = atoi(strchr(command, ' ') + 1);
  if (position < 0 || position >= positionsNum) {
    Serial.println(F("CMD ERROR INVALIDPOSITION"));
    return;
  }

  /* Acknowledge command: */
  Serial.println(F("CMD OK"));

  /* Set the new target: */
  targetPosition = position;
  stepper.moveTo(positionMin + position * stepsPerPosition);
  state_set(F("TARGET"), position);

  /* Set positionReached: */
  targetPositionReached = false;
}


void cmd_pump(char *command) {
  /* Extract value: */
  int val = atoi(strchr(command, ' ') + 1);
  if (val < 0 || val >= pumpsNum) {
    Serial.println(F("CMD ERROR INVALIDVALUE"));
    return;
  }

  /* Acknowledge command: */
  Serial.println(F("CMD OK"));

  /* Switch pump: */
  if (val == 0) {
    pumpOff();
  } else {
    pumpOn(val, 0);
  }
}


void parseCommand(char *command) {
  if (strcmp(command, "HOME") == 0) {
    /* Go to home position. */
    Serial.println(F("CMD OK BLOCKING"));
    cmd_home();
  } else if (strstr(command, "GOTO ") == command) {
    /* Go to given position. */
    cmd_goto(command);
  } else if (strstr(command, "PUMP ") == command) {
    /* Start or stop the pump. */
    cmd_pump(command);
  } else {
    /* Unknown command. */
    Serial.println(F("CMD ERROR UNKNOWNCOMMAND"));
  }
}

int lastButtonValue = LOW; /* TODO: Remove this. */

void loop() {
  /* Run scheduled tasks: */
  taskScheduler.run();

  /* Run LEDs: */
  countLEDs.show();
  pumpLEDs.show();

  /* Check for button press / release: */
  int buttonValue = startButton.get();
  if (buttonValue != lastButtonValue) {
    state_set(F("BUTTON"), buttonValue);
    lastButtonValue = buttonValue;

    if (buttonValue == HIGH) {
      cmd_home();
      /* TODO */
    }
  }

  /* Check for selector changes: */
  if (countSelector.run()) {
    state_set(F("SELECTEDCOUNT"), countSelector.get());
    countLEDs.clear();
    countLEDs.fill(countLEDsColor, 0, countSelector.get() + 1);
  }
  if (pumpSelector.run()) {
    state_set(F("SELECTEDPUMP"), pumpSelector.get());
    pumpLEDs.clear();
    pumpLEDs.setPixelColor(pumpSelector.get(), pumpLEDsColor);
  }

  /* If endstop switch is pressed, stop instantly: */
  if (digitalRead(pinEndstop) == LOW) {
    stepper.stop();
    if (!endstopErrorPrinted) {
      Serial.println(F("MSG ERROR ENDSTOPUNEXPECTED"));
      endstopErrorPrinted = true;
    }
    /* Suppress POSREACHED message: */
    targetPositionReached = true;
    return;
  }

  /* If endstop is released, print a message once: */
  if (endstopErrorPrinted) {
    Serial.println(F("MSG INFO ENDSTOPRELEASED"));
    endstopErrorPrinted = false;
  }

  /* Parse new commands: */
  commandReader.run();

  /* Print a message when we arrived at our target: */
  if (!targetPositionReached && stepper.distanceToGo() == 0) {
    state_set(F("POSITION"), targetPosition);
    targetPositionReached = true;
  }

  /* Run the stepper motor: */
  stepper.run();
}
