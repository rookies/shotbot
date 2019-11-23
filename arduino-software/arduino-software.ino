#include <AccelStepper.h>
#include <Servo.h>
#include "Config.hh"
#include "TaskScheduler.hh"
#include "CommandReader.hh"

const long stepsPerPosition = (positionMax - positionMin) / (positionsNum - 1);
bool positionReached = true;
bool endstopErrorPrinted = false;


void parseCommand(char *);


AccelStepper stepper(AccelStepper::DRIVER, pinStep, pinDirection);
Servo valveServo;
TaskScheduler<4> taskScheduler;
CommandReader<commandLengthMax> commandReader(parseCommand);


/* Task IDs: */
const size_t taskId_closeServoValve = 0;
const size_t taskId_switchOffValveServo = 1;
const size_t taskId_closeSolenoidValve = 2;
const size_t taskId_switchOffPump = 3;


/* HW Control: */
void servoOn() {
  digitalWrite(pinValveServoEnable, HIGH);
  Serial.println(F("STATE SERVO ON"));
}
void servoOff() {
  digitalWrite(pinValveServoEnable, LOW);
  Serial.println(F("STATE SERVO OFF"));
}
void solenoidOpen() {
  digitalWrite(pinSolenoidValve, HIGH);
  Serial.println(F("STATE SOLENOID OPEN"));
}
void solenoidClose() {
  digitalWrite(pinSolenoidValve, LOW);
  Serial.println(F("STATE SOLENOID CLOSED"));
}
void valveOpen() {
  servoOn();
  valveServo.write(valveAngleOpen);
  Serial.println(F("STATE VALVE OPEN"));

  /* Switch off servo after some delay: */
  taskScheduler.scheduleTask(taskId_switchOffValveServo, valveServoOffDelay);
}
void valveClose() {
  servoOn();
  valveServo.write(valveAngleClosed);
  Serial.println(F("STATE VALVE CLOSED"));

  /* Switch off servo after some delay: */
  taskScheduler.scheduleTask(taskId_switchOffValveServo, valveServoOffDelay);
}
void pumpOn() {
  digitalWrite(pinPump, HIGH);
  Serial.println(F("STATE PUMP ON"));

  /* Schedule switching off the pump after pumpMaxTime: */
  taskScheduler.scheduleTask(taskId_switchOffPump, pumpMaxTime);
}
void pumpOff() {
  digitalWrite(pinPump, LOW);
  Serial.println(F("STATE PUMP OFF"));

  /* Release pressure: */
  solenoidOpen();
  taskScheduler.scheduleTask(taskId_closeSolenoidValve, pressureReleaseTime);

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

  /* Setup pump: */
  pinMode(pinPump, OUTPUT);
  pumpOff();

  /* Setup valve servo: */
  valveServo.attach(pinValveServo);
  pinMode(pinValveServoEnable, OUTPUT);
  valveClose();

  /* Setup solenoid valve: */
  pinMode(pinSolenoidValve, OUTPUT);
  solenoidClose();

  /* Setup tasks: */
  taskScheduler.setTask(taskId_closeServoValve, valveClose);
  taskScheduler.setTask(taskId_switchOffValveServo, servoOff);
  taskScheduler.setTask(taskId_closeSolenoidValve, solenoidClose);
  taskScheduler.setTask(taskId_switchOffPump, pumpOff);

  /* Report that we're ready: */
  Serial.print(F("READY POSITIONS "));
  Serial.println(positionsNum);
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

  /* Inform that we finished: */
  Serial.println(F("CMD FINISHED"));

  /* Suppress POSREACHED message: */
  positionReached = true;
}


void cmd_goto(char *command) {
  /* Extract position: */
  int position = atoi(strchr(command, ' ') + 1);
  if (position < 0 || position >= positionsNum) {
    Serial.println(F("CMD ERROR INVALIDPOSITION"));
    return;
  }

  /* Acknowledge command: */
  Serial.println(F("CMD OK"));

  /* Set the new target: */
  stepper.moveTo(positionMin + position * stepsPerPosition);
  Serial.print(F("STATE TARGET "));
  Serial.println(position);

  /* Set positionReached: */
  positionReached = false;
  Serial.println(F("STATE TARGETREACHED FALSE"));
}


void cmd_valve(char *command) {
  /* Extract time: */
  int time = atoi(strchr(command, ' ') + 1);
  if (time < 0 || time > valveMaxTimeOpen) {
    Serial.println(F("CMD ERROR INVALIDTIME"));
    return;
  }

  /* Acknowledge command: */
  Serial.println(F("CMD OK"));

  /* Open or close the valve: */
  if (time > 0) {
    valveOpen();
    /* Schedule closing the valve: */
    taskScheduler.scheduleTask(taskId_closeServoValve, time);
  } else {
    valveClose();
  }
}


void cmd_pump(char *command) {
  /* Extract value: */
  int val = atoi(strchr(command, ' ') + 1);
  if (val != 0 && val != 1) {
    Serial.println(F("CMD ERROR INVALIDVALUE"));
    return;
  }

  /* Acknowledge command: */
  Serial.println(F("CMD OK"));

  /* Switch pump: */
  if (val == 0) {
    pumpOff();
  } else if (val == 1) {
    pumpOn();
  }
}


void parseCommand(char *command) {
  if (strcmp(command, "HOME") == 0) {
    /* Go to home position. */
    cmd_home(command);
  } else if (strstr(command, "GOTO ") == command) {
    /* Go to given position. */
    cmd_goto(command);
  } else if (strstr(command, "VALVE ") == command) {
    /* Open the valve for a given time or close it. */
    cmd_valve(command);
  } else if (strstr(command, "PUMP ") == command) {
    /* Start or stop the pump. */
    cmd_pump(command);
  } else {
    /* Unknown command. */
    Serial.print(F("CMD ERROR UNKNOWNCOMMAND"));
  }
}


void loop() {
  /* Run scheduled tasks: */
  taskScheduler.run();

  /* If endstop switch is pressed, stop instantly: */
  if (digitalRead(pinEndstop) == LOW) {
    stepper.stop();
    if (!endstopErrorPrinted) {
      Serial.println(F("ERROR ENDSTOPUNEXPECTED"));
      endstopErrorPrinted = true;
    }
    /* Suppress POSREACHED message: */
    positionReached = true;
    return;
  }

  /* If endstop is released, print a message once: */
  if (endstopErrorPrinted) {
    Serial.println(F("INFO ENDSTOPRELEASED"));
    endstopErrorPrinted = false;
  }

  /* Parse new commands: */
  commandReader.run();

  /* Print a message when we arrived at our target: */
  if (!positionReached && stepper.distanceToGo() == 0) {
    Serial.println(F("STATE TARGETREACHED TRUE"));
    positionReached = true;
  }

  /* Run the stepper motor: */
  stepper.run();
}
