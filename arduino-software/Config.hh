#pragma once

/* Serial console */
const long serialBaudrate = 38400;
const size_t commandLengthMax = 20;

/* Pins */
const uint8_t pinEndstop = 2; /* low active */
const uint8_t pinStep = 3;
const uint8_t pinDirection = 4;
const uint8_t pinEnable = 5;
const uint8_t pumpsNum = 2;
const uint8_t pinPumps[] = {
  7, /* previously used for air pump */
  6, /* previously used for solenoid valve */
};

/* Stepper */
const float maxStepsPerSecond = 3200;
const float homeStepsPerSecond = 1000;
const float acceleration = 1000;
const long positionMin = 400;
const long positionMax = 26000;
const uint8_t positionsNum = 6;

/* Pump */
const long pumpMaxTime = 120000; /* ms */
