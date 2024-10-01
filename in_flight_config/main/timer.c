#include "freertos/FreeRTOS.h"
#define Time int64_t

Time start_timer(){
	return esp_timer_get_time();
}

// time since "timer" in seconds
float query_timer_seconds(int64_t time){
	int64_t current_time = esp_timer_get_time();
	return 0.000001*(float)(current_time - time);
}

int64_t query_timer_microseconds(int64_t time){
	return esp_timer_get_time() - time;
}

Time start_time_for_uptime = 0;

void init_uptime(){
	start_time_for_uptime = start_timer();
}

float get_uptime_seconds(){
	return query_timer_seconds(start_time_for_uptime);
}

float get_time_step(Time* t){
	if(*t == 0) {*t = start_timer(); return 0;}
	float d_t = query_timer_seconds(*t);
	*t = start_timer();
	return d_t;
}
