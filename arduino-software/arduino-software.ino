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


void home(bool melody=false) {
  /* Run backwards until the endstop switch is pressed: */
  stepper.setSpeed(-homeStepsPerSecond);
  while (digitalRead(pinEndstop) == HIGH) {
    stepper.runSpeed();
  }
  stepper.stop();

  /* Zero the position: */
  stepper.setCurrentPosition(0);

  /* Play a short melody to signal that we're ready: */
  if (melody) {
    tone(pinStep, 8372); // C
    delay(500);
    tone(pinStep, 10548); // E
    delay(500);
    tone(pinStep, 12544); // G
    delay(500);
    noTone(pinStep);
  }

  /* Go to minimal position: */
  stepper.runToNewPosition(positionMin);
}


/* Task IDs: */
const size_t taskId_closeServoValve = 0;
const size_t taskId_switchOffValveServo = 1;
const size_t taskId_closeSolenoidValve = 2;
const size_t taskId_switchOffPump = 3;

/* Tasks: */
void closeServoValve() {
  digitalWrite(pinValveServoEnable, HIGH);
  valveServo.write(valveAngleClosed);
  Serial.println(F("INF VALVECLOSED"));
  /* Switch off servo after some delay: */
  taskScheduler.scheduleTask(taskId_switchOffValveServo, valveServoOffDelay);
}
void switchOffValveServo() {
  digitalWrite(pinValveServoEnable, LOW);
  /* TODO: Add message? */
}
void closeSolenoidValve() {
  digitalWrite(pinSolenoidValve, LOW);
  /* TODO: Add message? */
}
void switchOffPump() {
  digitalWrite(pinPump, LOW);
  digitalWrite(pinSolenoidValve, HIGH);
  taskScheduler.scheduleTask(taskId_closeSolenoidValve, pressureReleaseTime);
  /* TODO: Add message? */
}


void setup() {
  /* Setup serial port: */
  Serial.begin(serialBaudrate);

  /* Setup stepper motor: */
  stepper.setEnablePin(pinEnable);
  stepper.setPinsInverted(false, false, true);
  stepper.enableOutputs();
  stepper.setMaxSpeed(maxStepsPerSecond);
  stepper.setAcceleration(acceleration);

  /* Setup endstop switch: */
  pinMode(pinEndstop, INPUT);

  /* Setup pump: */
  pinMode(pinPump, OUTPUT);
  digitalWrite(pinPump, LOW);

  /* Setup valve servo: */
  valveServo.attach(pinValveServo);
  pinMode(pinValveServoEnable, OUTPUT);
  digitalWrite(pinValveServoEnable, HIGH);
  valveServo.write(valveAngleClosed);
  taskScheduler.scheduleTask(taskId_switchOffValveServo, valveServoOffDelay);

  /* Setup solenoid valve: */
  pinMode(pinSolenoidValve, OUTPUT);
  digitalWrite(pinSolenoidValve, LOW);

  /* Setup tasks: */
  taskScheduler.setTask(taskId_closeServoValve, closeServoValve);
  taskScheduler.setTask(taskId_switchOffValveServo, switchOffValveServo);
  taskScheduler.setTask(taskId_closeSolenoidValve, closeSolenoidValve);
  taskScheduler.setTask(taskId_switchOffPump, switchOffPump);

  /* Report that we're ready: */
  Serial.print(F("INF READY POSNUM "));
  Serial.println(positionsNum);
}

void parseCommand(char *command) {
  if (strcmp(command, "HOME") == 0) {
    /* Go to home position. */
    Serial.println(F("CMD HOME BLOCKING"));
    home(); /* TODO: Add melody parameter. */
    Serial.println(F("INF HOMED"));
    /* Suppress POSREACHED message: */
    positionReached = true;
  } else if (strstr(command, "GOTO ") == command) {
    /* Go to given position. */
    int position = atoi(strchr(command, ' ') + 1);
    if (position < 0 || position >= positionsNum) {
      Serial.print(F("ERR INVALIDPOS "));
      Serial.println(position);
    } else {
      stepper.moveTo(positionMin + position * stepsPerPosition);
      Serial.print(F("CMD GOTO NONBLOCKING "));
      Serial.println(position);
      positionReached = false;
    }
  } else if (strstr(command, "VALVE ") == command) {
    /* Open the valve for a given time or close it. */
    int time = atoi(strchr(command, ' ') + 1);
    if (time < 0 || time > valveMaxTimeOpen) {
      Serial.print(F("ERR INVALIDTIME "));
      Serial.println(time);
    } else {
      if (time > 0) {
        digitalWrite(pinValveServoEnable, HIGH);
        valveServo.write(valveAngleOpen);
      }
      taskScheduler.scheduleTask(taskId_closeServoValve, time);
      Serial.print(F("CMD VALVE NONBLOCKING "));
      Serial.println(time);
    }
  } else if (strstr(command, "PUMP ") == command) {
    /* Start or stop the pump. */
    int val = atoi(strchr(command, ' ') + 1);
    if (val == 0) {
      switchOffPump();

      /* Unschedule switching off the pump after pumpMaxTime: */
      taskScheduler.unscheduleTask(taskId_switchOffPump);

      Serial.println(F("CMD PUMP DONE 0"));
    } else if (val == 1) {
      digitalWrite(pinPump, HIGH);

      /* Schedule switching off the pump after pumpMaxTime: */
      taskScheduler.scheduleTask(taskId_switchOffPump, pumpMaxTime);

      Serial.println(F("CMD PUMP DONE 1"));
    } else {
      Serial.print(F("ERR INVALIDVAL "));
      Serial.println(val);
    }
  } else {
    Serial.print(F("ERR UNKNOWNCMD "));
    Serial.println(command);
  }
}

void loop() {
  /* Run scheduled tasks: */
  taskScheduler.run();

  /* If endstop switch is pressed, stop instantly: */
  if (digitalRead(pinEndstop) == LOW) {
    stepper.stop();
    if (!endstopErrorPrinted) {
      Serial.println(F("ERR ENDSTOPUNEXPECTED"));
      endstopErrorPrinted = true;
    }
    /* Suppress POSREACHED message: */
    positionReached = true;
    return;
  } else if (endstopErrorPrinted) {
    Serial.println(F("INF ENDSTOPRELEASED"));
    endstopErrorPrinted = false;
  }

  /* Parse new commands: */
  commandReader.run();

  /* Print a message when we arrived at our target: */
  if (!positionReached && stepper.distanceToGo() == 0) {
    Serial.println(F("INF POSREACHED"));
    positionReached = true;
  }

  /* Run the stepper motor: */
  stepper.run();
}
