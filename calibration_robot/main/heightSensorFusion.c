float height_accelerometer = 0;
float height_velocity_accelerometer = 0;

float height_bmp280_smoothed = 0;
float last_height_bmp280_smoothed = 0;
float height_velocity_bmp280_smoothed = 0;
float time_difference_for_bmp280 = 0;

//for debugging
float hf_raw_height = 0;
float d_hf_raw_height = 0;

void initHeightSensorFusion(){
	initBMP280();
	startPressureMeasurement();
}
int pressureMeasurementCounter = 1;
void fuseHeightSensorData(){
	// calculating height from accelerometer data
	height_velocity_accelerometer += (accel_norm_smooth - mpu_pos_avg.accel_norm) * time_difference;//acc_height*time_difference;
	//height_velocity_accelerometer = 0.996*height_velocity_accelerometer + 0.004* height_velocity_bmp280_smoothed;
	//TODO (DONE): more influence of bmp280 when accelerations are high. Less influence of bmp280 when accelerations are low
	
	//TODO: here we could calculate a very Smooth exponential Average SAAa of Abs(accel_norm_smooth - mpu_pos_avg.accel_norm) and then use
	// initialization outside of this function: SAAa = 0;
	// SAAa = 0.99*SAAa + 0.01*fabs(accel_norm_smooth - mpu_pos_avg.accel_norm)*0.004 // factor 0.004 to get the effect of the right order of magnitude below
	// if (SAAa < 0) SAAa = 0;
	// if (SAAa > 3) SAAa = 3;
	// height_velocity_accelerometer = (0.997-SAAa)*height_velocity_accelerometer + (0.003+SAAa)* height_velocity_bmp280_smoothed;
	// this whole procedure would be advantageous because it assures that we correct the velocity sufficiently after accumulating a high error caused by too large accelerations.
	if(accel_norm_smooth - mpu_pos_avg.accel_norm > 2 || accel_norm_smooth - mpu_pos_avg.accel_norm < -2)
		height_velocity_accelerometer = 0.988*height_velocity_accelerometer + 0.012* height_velocity_bmp280_smoothed;
	else
		height_velocity_accelerometer = 0.997*height_velocity_accelerometer + 0.003* height_velocity_bmp280_smoothed;
	d_hf_raw_height += (accel_norm_smooth - mpu_pos_avg.accel_norm) * time_difference;
	
	time_difference_for_bmp280 += time_difference;
	
	height_accelerometer += height_velocity_accelerometer * time_difference;
	//height_accelerometer = 0.995*height_accelerometer + 0.005*(-10*getPressureDiff());//height_bmp280_smoothed;
	height_accelerometer = 0.98*height_accelerometer + 0.02*(-10*getPressureDiff());//height_bmp280_smoothed;
	hf_raw_height += height_velocity_accelerometer * time_difference; // for debugging
	
	if(pressureMeasurementCounter == 7){ // in every seventh call to fuseHeightSensorData()
		
		pressureMeasurementCounter = 0;
		
		readPressureMeasurement();
		readTemperatureMeasurement(); // TEMPERATURE
		startPressureMeasurement();
		
		last_height_bmp280_smoothed = height_bmp280_smoothed;
		height_bmp280_smoothed = -10*getPressureDiff();//0.9*height_bmp280_smoothed + 0.1*(-10*getPressureDiff());
		
		//TODO (DONE): height_velocity_bmp280_smoothed doesn't seem right. appears always positive and too small. might be too heavily smoothed
		height_velocity_bmp280_smoothed = 0.9*height_velocity_bmp280_smoothed + 0.1*(height_bmp280_smoothed-last_height_bmp280_smoothed)/time_difference_for_bmp280;
		time_difference_for_bmp280 = 0;
	}
	pressureMeasurementCounter++;
}

float getHeight(){
	return height_accelerometer;
}
