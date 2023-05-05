float kiteSpeed = 0;

void updateKiteSpeed(){
	if(fabs(rot0) > 0.9){ // kite flies upwards (x-coordinate of x-axis larger than 0.9, approx. 25 deg)
		kiteSpeed = 0.9999 * kiteSpeed + 0.0001 * height_velocity_accelerometer*fabs(rot0);
	}
	//TODO use sidewaysAngle
}

float getKiteSpeed(){
	return kiteSpeed*TWR;
}

float getLineForce(){
	return kiteSpeed*kiteSpeed*liftCoeffTimesArea;
}
