#include <Adafruit_NeoPixel.h>
#include "Config.hh"
#include "CommandReader.hh"
#include "SelectorKnob.hh"
#include "DebouncedSwitch.hh"
#include "State.hh"
#include "Hardware.hh"
#include "ServingStateMachine.hh"


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
ServingStateMachine servingMachine;


void setup() {
  /* Setup hardware: */
  Serial.begin(serialBaudrate);
  Stepper::getInstance(); /* TODO: Prevent this from being optimized away? */
  Pump::getInstance(); /* TODO: Prevent this from being optimized away? */
  countLEDs.begin();
  pumpLEDs.begin();
  countLEDs.show();
  pumpLEDs.show();

  /* Report that we're ready: */
  state_set(F("READY"), true);
  state_set(F("POSITIONS"), positionsNum);
  state_set(F("PUMPS"), pumpsNum);
}


void parseCommand(char *command) {
  if (strcmp(command, "HOME") == 0) {
    /* ********** HOME ********** */
    /* Go to home position. */
    Serial.println(F("CMD OK BLOCKING"));
    Stepper::getInstance().home();

  } else if (strstr(command, "GOTO ") == command) {
    /* ********** GOTO ********** */
    /* Go to given position. */
    int position = atoi(strchr(command, ' ') + 1);
    Stepper::getInstance().move(position);

  } else if (strstr(command, "PUMP ") == command) {
    /* TODO: Fix this command. */
    /* ********** PUMP ********** */
    /* Switch on / off the pump. */
    int val = atoi(strchr(command, ' ') + 1);
    if (val < 0 || val >= pumpsNum) {
      Serial.println(F("CMD ERROR INVALIDVALUE"));
      return;
    }
    Serial.println(F("CMD OK"));
    if (val == 0) {
      Pump::getInstance().off();
    } else {
      Pump::getInstance().on(val, 0);
    }

  } else {
    /* Unknown command. */
    Serial.println(F("CMD ERROR UNKNOWNCOMMAND"));
  }
}


int lastButtonValue = LOW; /* TODO: Remove this. */

void loop() {
  /* Run hardware: */
  Pump::getInstance().run();
  Stepper::getInstance().run();

  /* Check for button press / release: */
  int buttonValue = startButton.get();
  if (buttonValue != lastButtonValue) {
    state_set(F("BUTTON"), buttonValue);
    lastButtonValue = buttonValue;

    if (buttonValue == HIGH) {
      Stepper::getInstance().home();
      servingMachine.start(pumpSelector.get(), countSelector.get() + 1);
      /* TODO: What to do if it's already running? Abort? */
    }
  }

  /* Check for selector changes: */
  if (countSelector.run()) {
    state_set(F("SELECTEDCOUNT"), countSelector.get());
    countLEDs.clear();
    countLEDs.fill(countLEDsColor, 0, countSelector.get() + 1);
    countLEDs.show();
  }
  if (pumpSelector.run()) {
    state_set(F("SELECTEDPUMP"), pumpSelector.get());
    pumpLEDs.clear();
    pumpLEDs.setPixelColor(pumpSelector.get(), pumpLEDsColor);
    pumpLEDs.show();
  }

  /* Parse new commands: */
  commandReader.run();

  /* Run state machine: */
  servingMachine.run();
}
