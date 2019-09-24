#include <AccelStepper.h>
#include "Config.hh"

const long stepsPerPosition = (positionMax - positionMin) / (positionsNum - 1);
char command[commandLengthMax+1];
uint8_t commandLength = 0;
bool positionReached = true;
bool endstopErrorPrinted = false;
unsigned long pumpFinished = 0;

AccelStepper stepper(AccelStepper::DRIVER, pinStep, pinDirection);

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
  } else if (strstr(command, "PUMP ") == command) {
    /* Activate the pump for a given time or deactivate it. */
    int time = atoi(strchr(command, ' ') + 1);
    if (time < 0 || time > pumpMaxTime) {
      Serial.print(F("ERR INVALIDTIME "));
      Serial.println(time);
    } else {
      if (time > 0) {
        digitalWrite(pinPump, HIGH);
      }
      pumpFinished = millis() + time;
      Serial.print(F("CMD PUMP NONBLOCKING "));
      Serial.println(time);
    }
  } else {
    Serial.print(F("ERR UNKNOWNCMD "));
    Serial.println(command);
  }
}

void loop() {
  /* Stop the pump if the time elapsed: */
  if (pumpFinished > 0 && millis() >= pumpFinished) {
    digitalWrite(pinPump, LOW);
    pumpFinished = 0;
    Serial.println(F("INF PUMPFINISHED"));
  }
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
  /* Check for new commands: */
  while (Serial.available() > 0) {
    char chr = Serial.read();
    if (chr != '\n' && chr != '\r' && commandLength < commandLengthMax) {
      /* Save received char in our buffer: */
      command[commandLength++] = chr;
    } else {
      /* Command finished, add null byte and prepare for next one: */
      command[commandLength] = '\0';
      commandLength = 0;
      /* ... and parse it: */
      parseCommand(command);
    }
  }
  /* Print a message if we arrived at our target: */
  if (!positionReached && stepper.distanceToGo() == 0) {
    Serial.println(F("INF POSREACHED"));
    positionReached = true;
  }
  /* Run the motor: */
  stepper.run();
}
