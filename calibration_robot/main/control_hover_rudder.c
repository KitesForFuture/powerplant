// PID CONTROLLER FOR KEEPING THE Y-AXIS LEVEL (Z-AXIS CONTROLLER)
float P_z = 0;
float D_z = 0;
float hover_rudder_control(){
	// rot is the rotation matrix of the drone in space. last column (z-axis) and 
	// z-values of the rotated x and y axes are accurate to 0.03 degrees, other values drift by 1 degree per minute. rot is an array of dimension 9, rot[6] is 3rd (2nd) row, 1st (0th) column.
	
	// calculate angle around z-axis
	// vector a is where the y-axis should be
	// a lies in the horizontal plane and in the plane of the wing. it is the position nearest to the y-axis of the kite in terms of control
	float a[3];
	crossProduct(rot2, rot5, rot8, 1, 0, 0, a);
	
	// this factor ensures that when it is horizontal like a plane, i.e. when the neutral position is undefined, it doesn't control
	float factor = normalize(a, a, 3);
	float y[3];
	y[0] = rot1;
	y[1] = rot4;
	y[2] = rot7;
	// angleDifference lies between 0 and 180 degrees, i.e. 0 and pi.
	float angleDifference = safe_acos(scalarProductOfMatrices(a, y, 3));
	
	// getting a sign on the angle:
	float b[3];
	// z cross a is a turned 90 degrees in the wing plane
	crossProduct(rot2, rot5, rot8, a[0], a[1], a[2], b);
	float tmp_variable = scalarProductOfMatrices(b, y, 3);
	angleDifference *= ((tmp_variable > 0) - (tmp_variable < 0));
	
	P_z = factor*(206.5 * angleDifference /* + LinksRechtsOffset*/);
	// D_z is in degree per second
	// D_z STAYS ACTIVE EVEN WHEN KITE IS HORIZONTAL
	D_z = gyroz - avgGyroz;
	
	return Dz*D_z - /*(1.0-angleDifference)**/1.2*Pz*P_z + SERVO_RUDDER_ZERO_ANGLE;
}
	
