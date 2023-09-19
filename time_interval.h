#ifndef __TIME_INTERVAL_H__
#define __TIME_INTERVAL_H__

#include <time.h>

typedef struct time_interval_t {
  double last_time;
  double interval;
} time_interval_t;

// Initialize a time interval with a specified interval
time_interval_t init_time_interval(double seconds) {
  time_interval_t interval;
  interval.last_time = GetTime();
  interval.interval = seconds;
  return interval;
}

// Check if the time interval has elapsed and reset the timer if it has
bool check_time_interval(time_interval_t *timer) {
  double current_time = GetTime();
  if (current_time - timer->last_time >= timer->interval) {
    timer->last_time = current_time;
    return true;
  }
  return false;
}

#endif