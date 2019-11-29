#pragma once

/* Serial console */
const long serialBaudrate = 86400;
const size_t commandLengthMax = 20;

/* Pins */
const uint8_t pinEndstop = 2; /* low active */
const uint8_t pinStep = 3;
const uint8_t pinDirection = 4;
const uint8_t pinEnable = 5;
const uint8_t pinSolenoidValve = 6;
const uint8_t pinPump = 7;
const uint8_t pinValveServo = 8;
const uint8_t pinValveServoEnable = 9;

/* Stepper */
const float maxStepsPerSecond = 3200;
const float homeStepsPerSecond = 1000;
const float acceleration = 1000;
const long positionMin = 400;
const long positionMax = 26000;
const uint8_t positionsNum = 6;

/* Pump */
const long pumpMaxTime = 120000; /* ms */

/* Valve Servo */
const int valveMaxTimeOpen = 10000; /* ms */
const uint8_t valveAngleClosed = 160;
const uint8_t valveAngleOpen = 60;
const int valveServoOffDelay = 1000; /* ms */

/* Solenoid Valve */
const int pressureReleaseTime = 1000; /* ms */
