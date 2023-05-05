#define HOVER_MODE 0
#define GLIDE_MODE 1
#define MOTORPLANE_MODE 2

int flightMode = GLIDE_MODE;

void gotoHoverMode(){
	if(flightMode == GLIDE_MODE){
		flightMode = HOVER_MODE;
		targetHeight = getHeight();
	}
}

void gotoGlideMode(){
	flightMode = GLIDE_MODE;
}

float servoRudder = 0;
float servoElevator = 0;
float motorLeft = 0;
float motorRight = 0;

void calculatePID(){

	// MOTORS
	/*
	// VERY IMPORTANT to keep C_z effective (necessary only if z-axis is being controlled/stabilized by motor thrust):
	if(C_h > (MOTOR_MAX_SPEED - 10)*(1-angleDifference)) C_h = (MOTOR_MAX_SPEED - 10)*(1-angleDifference) + MOTOR_SPEED_WHEN_HORIZONTAL*angleDifference;
	if(C_h < 0) C_h = 0;
	*/
	
	motorLeft = flightMode == HOVER_MODE ? hover_height_control() : 0;
	motorRight = motorLeft;
	
	//TODO: power mode, haha :D - Use with care!!!!!
	/*
	if(fabs(signal2) > 0.3 && flightMode != GLIDE_MODE ){
		motorLeft =  90.;
		motorRight = motorLeft;
	}
	*/
	
	// MIXING PID FOR Z-AXIS AND X-AXIS
	servoRudder = (flightMode == GLIDE_MODE || flightMode == MOTORPLANE_MODE) ? glide_rudder_control() : hover_rudder_control();
	servoElevator = (flightMode == GLIDE_MODE || flightMode == MOTORPLANE_MODE) ? glide_elevator_control() : hover_elevator_control();
	
	
	
}
