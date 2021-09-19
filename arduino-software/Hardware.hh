#pragma once
#include <AccelStepper.h>
#include "Config.hh"
#include "State.hh"

/* This is a singleton. */
class Pump {
  public:
    static Pump& getInstance() {
      static Pump instance;
      return instance;
    }

    Pump(Pump const&) = delete;
    void operator=(Pump const&) = delete;

    void on(uint8_t i, long duration) {
      if (i >= pumpsNum) return;

      if (duration <= 0 || duration > pumpMaxTime) {
        duration = pumpMaxTime;
      }

      digitalWrite(pinPumps[i], HIGH);
      state_set_onoff(F("PUMP"), i, true, duration);

      /* Schedule switching off the pump: */
      m_on = true;
      m_switchOffTime = millis() + duration;
      m_currentlyActivePump = i;
    }

    void off() {
      if (m_currentlyActivePump >= pumpsNum) return;

      digitalWrite(pinPumps[m_currentlyActivePump], LOW);
      state_set_onoff(F("PUMP"), m_currentlyActivePump, false);

      /* Unschedule switching off the pump: */
      m_on = false;
    }

    void run() {
      /* Checks if the pump has to be switched off and if so, switches it off. */
      if (m_on && millis() >= m_switchOffTime) {
        off();
      }
    }

    bool isOn() {
      return m_on;
    }
  private:
    Pump() {
      for (uint8_t i=0; i < pumpsNum; i++) {
        pinMode(pinPumps[i], OUTPUT);
        m_currentlyActivePump = i;
        off();
      }
      m_currentlyActivePump = -1;
    }

    uint8_t m_currentlyActivePump;
    bool m_on;
    unsigned long m_switchOffTime;
};


/* This is a singleton. */
class Stepper {
  public:
    static Stepper& getInstance() {
      static Stepper instance;
      return instance;
    }

    Stepper(Stepper const&) = delete;
    void operator=(Stepper const&) = delete;

    void on() {
      m_stepper.enableOutputs();
      state_set_onoff(F("STEPPER"), true);
    }

    void off() {
      m_stepper.disableOutputs();
      state_set_onoff(F("STEPPER"), false);

      /* Remember that we're not homed anymore: */
      m_homed = false;
      state_set(F("HOMED"), false);
    }

    void run() {
      /* If endstop switch is pressed, stop instantly: */
      if (digitalRead(pinEndstop) == LOW) {
        m_stepper.stop();
        if (!m_endstopErrorPrinted) {
          Serial.println(F("MSG ERROR ENDSTOPUNEXPECTED"));
          m_endstopErrorPrinted = true;
        }
        /* Suppress POSREACHED message: */
        m_targetPositionReached = true;
        return;
      }

      /* If endstop is released, print a message once: */
      if (m_endstopErrorPrinted) {
        Serial.println(F("MSG INFO ENDSTOPRELEASED"));
        m_endstopErrorPrinted = false;
      }

      /* Run the stepper motor: */
      m_stepper.run();

      /* Print a message when we arrived at our target: */
      if (!m_targetPositionReached && m_stepper.distanceToGo() == 0) {
        state_set(F("POSITION"), m_targetPosition);
        m_targetPositionReached = true;
      }
    }

    void home() {
      /* TODO: Make non-blocking? */
      state_set(F("BLOCKED"), true);

      /* Run backwards until the endstop switch is pressed: */
      m_stepper.setSpeed(-homeStepsPerSecond);
      while (digitalRead(pinEndstop) == HIGH) {
        m_stepper.runSpeed();
      }
      m_stepper.stop();

      /* Zero the position: */
      m_stepper.setCurrentPosition(0);

      /* Go to minimal position: */
      m_stepper.runToNewPosition(positionMin);

      /* Remember that we homed: */
      m_homed = true;
      state_set(F("HOMED"), true);

      /* Remember the current position: */
      m_targetPosition = 0;
      state_set(F("POSITION"), 0);

      /* Suppress POSREACHED message: */
      m_targetPositionReached = true;

      state_set(F("BLOCKED"), false);
    }

    void move(int position) {
      /* Check for valid position: */
      if (position < 0 || position >= positionsNum) {
        Serial.println(F("CMD ERROR INVALIDPOSITION"));
        return;
      }

      /* Check if we're homed: */
      if (!m_homed) {
        Serial.println(F("CMD ERROR NOTHOMED"));
        return;
      }

      /* Acknowledge command: */
      Serial.println(F("CMD OK"));

      /* Set the new target: */
      m_targetPosition = position;
      m_stepper.moveTo(positionMin + position * stepsPerPosition);
      state_set(F("TARGET"), position);

      /* Set positionReached: */
      m_targetPositionReached = false;
    }

    bool isHomed() {
      return m_homed;
    }

    bool targetReached() {
      return m_targetPositionReached;
    }

    static const long stepsPerPosition = (positionMax - positionMin) / (positionsNum - 1);
  private:
    Stepper() : m_homed(false), m_stepper(AccelStepper(AccelStepper::DRIVER, pinStep, pinDirection)),
                m_targetPosition(0), m_targetPositionReached(true), m_endstopErrorPrinted(false) {
      /* Setup stepper: */
      m_stepper.setEnablePin(pinEnable);
      m_stepper.setPinsInverted(false, false, true);
      m_stepper.setMaxSpeed(maxStepsPerSecond);
      m_stepper.setAcceleration(acceleration);
      on();

      /* Setup endstop switch: */
      pinMode(pinEndstop, INPUT);

      state_set(F("HOMED"), false);
      state_set(F("TARGET"), -1);
      state_set(F("POSITION"), -1);
      state_set(F("BLOCKED"), false);
    }

    AccelStepper m_stepper;
    bool m_homed;
    uint8_t m_targetPosition;
    bool m_targetPositionReached;
    bool m_endstopErrorPrinted;
};
