#pragma once

#include <Adafruit_NeoPixel.h>

/* Serial console */
const long serialBaudrate = 38400;
const size_t commandLengthMax = 20;

/* Number of positions, pumps, and LEDs */
const uint8_t positionsNum = 6;
const uint8_t pumpsNum = 2;
const uint8_t countLEDsNum = 8;
/* TODO: Check that positionsNum <= countLEDsNum */

/* Pins */
/* 3x ADC (selector knobs)
 * 10x Digital Out (3x stepper, 4x pumps, 2x LEDs, DFPlayer TX)
 * 3x Digital In (endstop, button, DFPlayer RX)
*/
const uint8_t pinEndstop = 5; /* low active */
const uint8_t pinStep = 3;
const uint8_t pinDirection = 4;
const uint8_t pinEnable = 2;
const uint8_t pinPumps[] = { 6, 7, 8, 9 };
const uint8_t pinCountSelector = A2;
const uint8_t pinPumpSelector = A1;
const uint8_t pinTimeSelector = A7; /* TODO */
const uint8_t pinCountLEDs = 11;
const uint8_t pinPumpLEDs = 10;
const uint8_t pinStartButton = 12;
const uint8_t pinDFPlayerTX = 13; /* TODO */
const uint8_t pinDFPlayerRX = A0; /* TODO */

/* Stepper */
const float maxStepsPerSecond = 10000;
const float homeStepsPerSecond = 2000;
const float acceleration = 2000;
const long positionMin = 800;
const long positionMax = 52000;

/* Pump */
const long pumpTime = 20000; /* ms */
const long pumpMaxTime = 120000; /* ms */

/* Selector knobs */
const int selectorMaxValue = 1023;
const int selectorHysteresis = 10;
/* TODO: More hysteresis with new stepper driver? */

/* LEDs */
const auto countLEDsColor = Adafruit_NeoPixel::Color(0, 20, 0);
const auto pumpLEDsColor = Adafruit_NeoPixel::Color(255, 255, 255);
