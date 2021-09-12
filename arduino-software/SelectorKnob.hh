#pragma once

/* Allows you to use an analog potentiometer as an input to select between
 * a defined number of values. Uses a configurable hysteresis to prevent
 * the selected value from bouncing back and forth. */

class SelectorKnob {
  public:
    SelectorKnob(uint8_t pin, uint8_t steps, int maxValue, int hysteresis) {
      m_pin = pin;
      m_hysteresis = hysteresis;

      m_lastValue = -1;

      /* Example mapping with steps = 3, maxValue = 24, hysteresis = 2:
       *              1           2
       *  0123456 78 9012345 67 8901234
       * |---0---|xx|---1---|xx|---2---|
       */
      m_stepSize = (maxValue + 1 - hysteresis * (steps - 1)) / steps;
    }

    bool run() {
      /* Get input value: */
      int inputValue = analogRead(m_pin);

      /* Calculate output value and check if it changed: */
      int fullStepSize = m_stepSize + m_hysteresis;
      int outputValue = inputValue / fullStepSize;
      if (outputValue == m_lastValue) {
        return false;
      }

      /* Check if it's in the “real” step range or the hysteresis range: */
      int offsetInsideStepRange = inputValue - outputValue * fullStepSize;
      if (offsetInsideStepRange < m_stepSize) {
        /* Inside the “real“ step range, i.e. a valid change */
        m_lastValue = outputValue;

        return true;
      }

      return false;
    }

    int get() {
      return m_lastValue;
    }

  private:
    uint8_t m_pin;
    int m_hysteresis;
    int m_lastValue;
    int m_stepSize;
};
