#pragma once
#include "Hardware.hh"

enum ServingStateMachine_State {
  UNINITIALIZED,
  START,
  PUMPING,
  MOVING,
};

class ServingStateMachine {
  public:
    ServingStateMachine() : m_state(UNINITIALIZED) {}

    void start(int pump, int count) {
      if (count < 1) {
        m_state = UNINITIALIZED;
        return;
      }

      m_state = START;
      m_pump = pump;
      m_count = count;
    }

    void run() {
      switch (m_state) {
        case START:
          m_iteration = 0;
          Pump::getInstance().on(m_pump, 1000); /* TODO: Make duration configurable. */
          m_state = PUMPING;
          break;
        case PUMPING:
          if (!Pump::getInstance().isOn()) {
            m_iteration++;

            if (m_iteration >= m_count) {
              Stepper::getInstance().move(0);
              m_state = UNINITIALIZED;
            } else {
              Stepper::getInstance().move(m_iteration);
              m_state = MOVING;
            }
          }
          break;
        case MOVING:
          if (Stepper::getInstance().targetReached()) {
            Pump::getInstance().on(m_pump, 1000); /* TODO: Make duration configurable. */
            m_state = PUMPING;
          }
          break;
        default:
          return;
      }
    }
  private:
    ServingStateMachine_State m_state;
    int m_pump;
    int m_count;
    int m_iteration;
};
