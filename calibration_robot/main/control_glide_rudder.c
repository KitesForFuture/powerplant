float LinksRechtsOffset = 0;

// DAMPING TERM for the rudder
float glide_rudder_control(){
	
	// D_z is in degree per second
	float D_z = gyroz - avgGyroz;
	float C_z = 0.5*Dz*D_z + SERVO_RUDDER_ZERO_ANGLE;
	// DAMPING should be less when a turn is intended
	if(fabs(LinksRechtsOffset) >= RUDDER_GLIDE_THRESHOLD){
		if(LinksRechtsOffset > 0.0){
			C_z = 0.1*Dz*D_z + SERVO_RUDDER_ZERO_ANGLE - LinksRechtsOffset + RUDDER_GLIDE_THRESHOLD;
		}else{
			C_z = 0.1*Dz*D_z + SERVO_RUDDER_ZERO_ANGLE - LinksRechtsOffset - RUDDER_GLIDE_THRESHOLD;
		}
	}
	return C_z;
}
