#include <AccelStepper.h>
#include <Servo.h>
#include "Config.hh"
#include "TaskScheduler.hh"
#include "CommandReader.hh"
#include "SelectorKnob.hh"

const long stepsPerPosition = (positionMax - positionMin) / (positionsNum - 1);
uint8_t targetPosition;
uint8_t currentlyActivePump = -1;
bool targetPositionReached = true;
bool endstopErrorPrinted = false;
bool homed = false;


void parseCommand(char *);


AccelStepper stepper(AccelStepper::DRIVER, pinStep, pinDirection);
TaskScheduler<1> taskScheduler;
CommandReader<commandLengthMax> commandReader(parseCommand);
SelectorKnob countSelector(pinCountSelector, positionsNum, selectorMaxValue,
                           selectorHysteresis);
SelectorKnob pumpSelector(pinPumpSelector, pumpsNum, selectorMaxValue,
                          selectorHysteresis);


/* Task IDs: */
const size_t taskId_switchOffPump = 0;


/* HW Control: */
void pumpOn(uint8_t i, long duration) {
  if (i >= pumpsNum) return;

  if (duration <= 0 || duration > pumpMaxTime) {
    duration = pumpMaxTime;
  }

  digitalWrite(pinPumps[i], HIGH);
  Serial.print(F("STATE PUMP ON "));
  Serial.print(i);
  Serial.print(F(" "));
  Serial.println(duration);

  /* Schedule switching off the pump after pumpMaxTime: */
  currentlyActivePump = i;
  taskScheduler.scheduleTask(taskId_switchOffPump, duration);
}
void pumpOff() {
  if (currentlyActivePump >= pumpsNum) return;

  digitalWrite(pinPumps[currentlyActivePump], LOW);
  Serial.print(F("STATE PUMP OFF "));
  Serial.println(currentlyActivePump);

  /* Unschedule switching off the pump after pumpMaxTime: */
  taskScheduler.unscheduleTask(taskId_switchOffPump);
}
void stepperOn() {
  stepper.enableOutputs();
  Serial.println(F("STATE STEPPER ON"));
}
void stepperOff() {
  stepper.disableOutputs();
  Serial.println(F("STATE STEPPER OFF"));

  /* Remember that we're not homed anymore: */
  homed = false;
  Serial.println(F("STATE HOMED FALSE"));
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

  /* Setup tasks: */
  taskScheduler.setTask(taskId_switchOffPump, pumpOff);

  /* We're not homed yet: */
  homed = false;
  Serial.println(F("STATE HOMED FALSE"));
  Serial.println(F("STATE TARGET -1"));
  Serial.println(F("STATE POSITION -1"));

  /* Report that we're ready: */
  Serial.print(F("READY POSITIONS "));
  Serial.print(positionsNum);
  Serial.print(F(" PUMPS "));
  Serial.println(pumpsNum);
}


void cmd_home(char *command) {
  /* Acknowledge command: */
  Serial.println(F("CMD OK BLOCKING"));

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
  Serial.println(F("STATE HOMED TRUE"));

  /* Remember the current position: */
  targetPosition = 0;
  Serial.println(F("STATE POSITION 0"));

  /* Suppress POSREACHED message: */
  targetPositionReached = true;
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
  Serial.print(F("STATE TARGET "));
  Serial.println(position);

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
    cmd_home(command);
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


void loop() {
  /* Run scheduled tasks: */
  taskScheduler.run();

  /* TODO */
  if (countSelector.run()) {
    Serial.print(F("Count: "));
    Serial.println(countSelector.get());
  }
  if (pumpSelector.run()) {
    Serial.print(F("Pump: "));
    Serial.println(pumpSelector.get());
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
    Serial.print(F("STATE POSITION "));
    Serial.println(targetPosition);
    targetPositionReached = true;
  }

  /* Run the stepper motor: */
  stepper.run();
}
