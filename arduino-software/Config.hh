#pragma once

/* Serial console */
const long serialBaudrate = 38400;
const size_t commandLengthMax = 20;

/* Number of positions, pumps, and LEDs */
const uint8_t positionsNum = 6;
const uint8_t pumpsNum = 2;
const uint8_t countLEDsNum = 8;
/* TODO: Check that positionsNum <= countLEDsNum */

/* Pins */
const uint8_t pinEndstop = 2; /* low active */
const uint8_t pinStep = 3;
const uint8_t pinDirection = 4;
const uint8_t pinEnable = 5;
const uint8_t pinPumps[] = {
  7, /* previously used for air pump */
  6, /* previously used for solenoid valve */
};
const uint8_t pinCountSelector = A0;
const uint8_t pinPumpSelector = A1;
const uint8_t pinTimeSelector = A2; /* TODO */
const uint8_t pinCountLEDs = 8;

/* Stepper */
const float maxStepsPerSecond = 3200;
const float homeStepsPerSecond = 1000;
const float acceleration = 1000;
const long positionMin = 400;
const long positionMax = 26000;

/* Pump */
const long pumpMaxTime = 120000; /* ms */

/* Selector knobs */
const int selectorMaxValue = 1023;
const int selectorHysteresis = 10;
