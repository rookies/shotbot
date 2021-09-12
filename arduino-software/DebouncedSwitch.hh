#pragma once

class DebouncedSwitch {
  public:
    DebouncedSwitch(uint8_t pin, bool pullup, unsigned long delay) {
      if (pullup) {
        pinMode(pin, INPUT_PULLUP);
      } else {
        pinMode(pin, INPUT);
      }

      m_pin = pin;
      m_delay = delay;

      m_lastValue = LOW;
      m_state = LOW;
      m_lastChangeTime = 0;
    }

    int get() {
      int value = digitalRead(m_pin);

      /* If the value changed from the last reading, reset timer: */
      if (value != m_lastValue) {
        m_lastChangeTime = millis();
      }

      /* If the reading stayed constant for at least the configured delay,
       * use the read value as our result: */
      if ((millis() - m_lastChangeTime) > m_delay) {
        m_state = value;
      }

      /* Remember our reading for the next call: */
      m_lastValue = value;

      return m_state;
    }

  private:
    uint8_t m_pin;
    unsigned long m_delay;
    int m_lastValue;
    int m_state;
    unsigned long m_lastChangeTime;
};
