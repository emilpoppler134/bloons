#include <time.h>
#include <stdbool.h>

#include "interval.h"

// Initialize a time interval with a specified interval
time_interval_t init_time_interval(double seconds) {
  time_interval_t interval;
  interval.last_time = time(NULL);;
  interval.interval = seconds;
  return interval;
}

// Check if the time interval has elapsed and reset the timer if it has
bool check_time_interval(time_interval_t *timer) {
  double current_time = time(NULL);;
  if (current_time - timer->last_time >= timer->interval) {
    timer->last_time = current_time;
    return true;
  }
  return false;
}