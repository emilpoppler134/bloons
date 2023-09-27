#include <time.h>
#include <stdbool.h>

#include "interval.h"

// Initialize a time interval with a specified interval
time_interval_t init_time_interval(double seconds) {
  time_t last_time;
  time(&last_time);
  double last_time_double = (double)last_time;

  time_interval_t interval;
  interval.last_time = last_time_double;
  interval.interval = seconds;
  return interval;
}

// Check if the time interval has elapsed and reset the timer if it has
bool check_time_interval(time_interval_t *timer) {
  time_t current_time;
  time(&current_time);
  double current_time_double = (double)current_time;
  
  if (current_time_double - timer->last_time >= timer->interval) {
    timer->last_time = current_time_double;
    return true;
  }
  
  return false;
}