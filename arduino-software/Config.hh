#pragma once

const long serialBaudrate = 86400;
const uint8_t pinStep = 4;
const uint8_t pinDirection = 5;
const uint8_t pinEnable = 6;
const uint8_t pinEndstop = A0; /* low active */
const float maxStepsPerSecond = 3200;
const float homeStepsPerSecond = 1000;
const float acceleration = 1000;
const long positionMin = 400;
const long positionMax = 29000;
const uint8_t positionsNum = 7;
const uint8_t commandLengthMax = 20;
