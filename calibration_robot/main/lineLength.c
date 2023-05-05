float lineLength = 0;
float averagedZx = 0;

void updateLineLength(){
	float tmp_height = getHeight();
	if(tmp_height > 1 && fabs(rot0) < 0.4){ // kite flies sideways (x-coordinate of x-axis smaller 0.4, approx. 25 deg)
		averagedZx = 0.9*averagedZx + 0.1*rot2; // x-coordinate of z-axis
		if(fabs(averagedZx) > 0.2){ // but at a certain angle to the ground
			lineLength = 0.9999*lineLength + 0.0001 * tmp_height/rot2;
		}
	}
}
