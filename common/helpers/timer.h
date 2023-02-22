
#ifndef HELPERS_TIMER
#define HELPERS_TIMER

#include "freertos/FreeRTOS.h"
#define Time int64_t

Time start_timer();
float query_timer_seconds(Time time);
int64_t query_timer_microseconds(Time time);
void init_uptime();
float get_uptime_seconds();

float get_time_step(Time* t);

#endif
