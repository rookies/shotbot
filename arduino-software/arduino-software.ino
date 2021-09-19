#include <Adafruit_NeoPixel.h>
#include "Config.hh"
#include "CommandReader.hh"
#include "SelectorKnob.hh"
#include "DebouncedSwitch.hh"
#include "State.hh"
#include "Hardware.hh"


void parseCommand(char *);


/* Inputs: */
SelectorKnob countSelector(pinCountSelector, positionsNum, selectorMaxValue,
                           selectorHysteresis);
SelectorKnob pumpSelector(pinPumpSelector, pumpsNum, selectorMaxValue,
                          selectorHysteresis);
DebouncedSwitch startButton(pinStartButton, true, 100);


/* Outputs: */
Adafruit_NeoPixel countLEDs(countLEDsNum, pinCountLEDs, NEO_KHZ800 + NEO_GRB);
Adafruit_NeoPixel pumpLEDs(pumpsNum, pinPumpLEDs, NEO_KHZ800 + NEO_GRB);


/* Utilities: */
CommandReader<commandLengthMax> commandReader(parseCommand);


void setup() {
  /* Setup serial port: */
  Serial.begin(serialBaudrate);

  /* Setup stepper motor: */
  Stepper::getInstance(); /* TODO: Prevent this from being optimized away? */

  /* Setup endstop switch: */
  pinMode(pinEndstop, INPUT);

  /* Setup pumps: */
  Pump::getInstance(); /* TODO: Prevent this from being optimized away? */

  /* Setup LEDs: */
  countLEDs.begin();
  pumpLEDs.begin();

  /* Report that we're ready: */
  state_set(F("READY"), true);
  state_set(F("POSITIONS"), positionsNum);
  state_set(F("PUMPS"), pumpsNum);
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
    Pump::getInstance().off();
  } else {
    Pump::getInstance().on(val, 0);
  }
}


void parseCommand(char *command) {
  if (strcmp(command, "HOME") == 0) {
    /* Go to home position. */
    Serial.println(F("CMD OK BLOCKING"));
    Stepper::getInstance().home();
  } else if (strstr(command, "GOTO ") == command) {
    /* Go to given position. */
    Stepper::getInstance().move(command);
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
  /* Check if pump needs to be switched off: */
  Pump::getInstance().run();

  /* Run LEDs: */
  countLEDs.show();
  pumpLEDs.show();

  /* Run stepper: */
  Stepper::getInstance().run();

  /* Check for button press / release: */
  int buttonValue = startButton.get();
  if (buttonValue != lastButtonValue) {
    state_set(F("BUTTON"), buttonValue);
    lastButtonValue = buttonValue;

    if (buttonValue == HIGH) {
      Stepper::getInstance().home();
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

  /* Parse new commands: */
  commandReader.run();
}
