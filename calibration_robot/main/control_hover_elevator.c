void setYAxisTrim(float trim){
	y_axis_trim = trim;
}


// PID CONTROLLER FOR KEEPING THE Z-AXIS LEVEL (Y-AXIS CONTROLLER)
float hover_elevator_control(){	
	
	// a is the desired position of the z(perp to airfoil)-axis (orthogonal to both the up-vector and the y-axis)
	float a[3];
	crossProduct(rot1, rot4, rot7, -1, 0, 0, a);
	
	// when props point sideways y-axis may be controlled by wind direction (... or better "controlled" through aerodynamic "longitudinal static stability" + remaining D_y term)
	// this factor ensures that when the props point sideways, it doesn't control
	float factor = normalize(a, a, 3);
	float z[3];
	z[0] = rot2;
	z[1] = rot5;
	z[2] = rot8;
	
	float angleDifference = safe_acos(scalarProductOfMatrices(a, z, 3));// is exactly the angle between the two vectors a and z.
	
	// getting a sign on the angle:
	float b[3];
	crossProduct(rot1, rot4, rot7, a[0], a[1], a[2], b); // a cross y, a pitched 90 degrees upwards
	float tmp_variable = scalarProductOfMatrices(b, z, 3); // tmp_variable positive when kite tilted backwards, negative, when tilted forward. (...modulo -1)
	angleDifference *= ((tmp_variable > 0) - (tmp_variable < 0));// <--- sign(<b,z>)
	
	// TODO: use pcb_y_axis_rotation_angle from constants.c, make it uppercase across whole code. and use BACKWARDS_TILT_ANGLE
	float P_y = 200*angleDifference;
	float D_y = gyroy-avgGyroy;
	//return 1.75*(-24 -0.12*Dy*D_y + factor*factor*(0.6*Py*P_y /*+ Iy*I_y*/ + 33));
	return 42 + 0.21*Dy*D_y + factor*factor*(-Py*P_y /*+ Iy*I_y*/ -57.75 + 10 + 76/*pcb glued in with slight forward pitch*/);
}
