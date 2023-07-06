float limitMotorForSafety(float motor){
	
	// LIMIT MOTOR SPEED FOR SAFETY
	if(motor < 0 ){
		return 0;
	}
	if(motor > MOTOR_MAX_SPEED){
		return MOTOR_MAX_SPEED;
	}
	return motor;
}

float limitRudderAngle(float rudder){
	
	if(rudder < SERVO_RUDDER_ZERO_ANGLE + SERVO_RUDDER_MIN_ANGLE)
		return SERVO_RUDDER_ZERO_ANGLE + SERVO_RUDDER_MIN_ANGLE;
	if(rudder > SERVO_RUDDER_ZERO_ANGLE + SERVO_RUDDER_MAX_ANGLE)
		return SERVO_RUDDER_ZERO_ANGLE + SERVO_RUDDER_MAX_ANGLE;
	return rudder;
}

float limitElevatorAngle(float elevator){
	
	if(elevator < SERVO_ELEVATOR_ZERO_ANGLE + SERVO_ELEVATOR_MIN_ANGLE)
		return SERVO_ELEVATOR_ZERO_ANGLE + SERVO_ELEVATOR_MIN_ANGLE;
	if(elevator > SERVO_ELEVATOR_ZERO_ANGLE + SERVO_ELEVATOR_MAX_ANGLE)
		return SERVO_ELEVATOR_ZERO_ANGLE + SERVO_ELEVATOR_MAX_ANGLE;
	return elevator;
}
