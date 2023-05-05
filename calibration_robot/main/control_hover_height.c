float lastHeight = 0;
float P_h; // for smoothing
float smooth_C_h = 0;

// used as a switch for positive, negative or neutral climb
float goalHeight = -5;
float targetHeight = -1;
float rateOfClimb = 0;

float oldGoalHeight = -5;

void setGoalHeight(float goalheight){
	goalHeight = goalheight;
	//targetHeight = getHeight();
}

void setRateOfClimb(float rate){
	rateOfClimb = fabs(rate);
}

// PID CONTROLLER FOR THE HEIGHT
float hover_height_control(){
	
	
	// letting targetHeight go towards goalHeight at rateOfClimb
	float adjustedClimbRate = rateOfClimb;
	if(targetHeight < 2) adjustedClimbRate = 0.2; // for descending slowly during the last 3 meters
	if(fabs(targetHeight - getHeight()) < 1){
		targetHeight += time_difference*adjustedClimbRate*goalHeight;
	}
	
	/*
	float adjustedClimbRate = rateOfClimb;
	if(fabs(goalHeight-targetHeight) > 0.1){
		if(targetHeight < 2) adjustedClimbRate = 0.2; // for descending slowly during the last 3 meters
		//if(goalHeight-targetHeight > 0) adjustedClimbRate = 2.0;
		targetHeight += time_difference*adjustedClimbRate*sign(goalHeight-targetHeight);
	}
	
	// exponential average smoothing of P_h
	//TODO: bound 3.5+(targetHeight - getHeight()) from below.
	*/
	
	P_h = 0.3*P_h + 0.7*(3.5+(targetHeight - getHeight()));
	
	// in meters per second
	// D_h, deviation from desired rate of climb
	// negative: climbing faster than needed
	// positive: climbing slower than needed
	
	float tmp_variable = 0;
	if(time_difference != 0){
		tmp_variable = (lastHeight - getHeight())/time_difference;
	}
	float D_h = (tmp_variable+sign(goalHeight-getHeight())*adjustedClimbRate);
	lastHeight = getHeight();
	
	float C_h = 0.7*(3*D_h + 10*P_h);
	
	// transition_factor is 1 when nose pointing to zenit, 0 when nose horizontal or lower
	float transition_factor = angle_nose_horizon() * 0.64;
	if(transition_factor < 0) transition_factor = 0;
	// (GRADUALLY) ...
	
	C_h = C_h*transition_factor + (1.0-transition_factor)*MOTOR_SPEED_WHEN_HORIZONTAL;
	
	// ... STOP CONTROLLING THE HEIGHT using the motors when angle to horizon is 0 or negative
	if(transition_factor < 0) C_h = 0.0;
	
	smooth_C_h = smooth_C_h * 0.9 + C_h * 0.1;
	
	return 3*smooth_C_h;
}
