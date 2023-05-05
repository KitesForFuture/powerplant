float HochRunterOffset = 0;

// increase longitudinal stability by a DAMPING TERM if the center of gravity is too far aft
float glide_elevator_control(){
	
	float D_y = gyroy-avgGyroy;
	float C_y = 0.35*Dy*D_y + SERVO_ELEVATOR_ZERO_ANGLE + ELEVATOR_GLIDE_NEUTRAL_ANGLE - HochRunterOffset;
	
	return limitElevatorAngle(C_y);
}
