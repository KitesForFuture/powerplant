#define Timer int64_t

int64_t startTimer(){
	return esp_timer_get_time();
}

// time since "timer" in seconds
float queryTimer(int64_t timer){
	int64_t currentTime = esp_timer_get_time();
	return 0.000001*(float)(currentTime - timer);
}

int64_t startTimeForUptime = 0;

void initUptime(){
	startTimeForUptime = startTimer();
}

float getUptime(){
	return queryTimer(startTimeForUptime);
}
