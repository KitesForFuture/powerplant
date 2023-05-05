// returns false=0 if right wing pointing up, otherwise returns true=1
int left_wing_pointing_up(){
	// if scalar product of y and (1,0,0) is positive, then left wing is pointing up
	return rot1 > 0 ? true : false;
}

// returns the signed angle between nose and horizon
// output is between -pi/2 and pi/2
// pi/2 ... nose points to sky
// -pi/2 ... nose points down
// 0 ... nose points to horizon
float angle_nose_horizon(){
	// acos(<plane_x_axis, (1,0,0)>)
	return 1.5708 - safe_acos(cos(pcb_y_axis_rotation_angle) * rot0 + sin(pcb_y_axis_rotation_angle) * rot2);
}
