#pragma once

/* TODO: What if millis() overflows? */
template<size_t N>
class TaskScheduler {
  public:
    TaskScheduler() {
      /* Disable all tasks: */
      for (size_t i=0; i < N; ++i) {
        _enabled[i] = false;
      }
    }

    void setTask(size_t idx, void (*task)(void)) {
      if (idx >= N) return;
      _tasks[idx] = task;
    }

    void scheduleTask(size_t idx, unsigned long delay) {
      if (idx >= N) return;
      _nextExecutions[idx] = millis() + delay;
      _enabled[idx] = true;
    }

    void unscheduleTask(size_t idx) {
      if (idx >= N) return;
      _enabled[idx] = false;
    }

    void run() {
      unsigned long now = millis();

      for (size_t i=0; i < N; ++i) {
        if (_enabled[i] && now >= _nextExecutions[i]) {
          (*_tasks[i])();
          _enabled[i] = false;
        }
      }
    }
  private:
    void (*_tasks[N])(void);
    bool _enabled[N];
    unsigned long _nextExecutions[N];
};
