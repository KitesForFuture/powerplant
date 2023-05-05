// HOW TO CALIBRATE:
// output acc_calibrationx,y,z preferably via wifi, set accel_offset_* in constants.c to the midpoints between highest and lowest reading.

struct position_data{
	float accel_norm;
	
	float accel_x;
	float accel_y;
	float accel_z;
	
	float gyro_x;
	float gyro_y;
	float gyro_z;
};

struct position_data mpu_pos;
struct position_data mpu_pos_avg;

float acc_x_smooth = 0;//mpu_pos.accel_x*accel_offset_x;// 1/mpu_pos_avg.accel_x to compensate for linear accelerometer inaccuracies.
float acc_y_smooth = 0;//mpu_pos.accel_y*accel_offset_y;// to be found in constants.c
float acc_z_smooth = 0;//mpu_pos.accel_z*accel_offset_z;
float accel_norm_smooth = 0;
float acc_height = 0;


// smoothed version of the acceleration vector, that's always pointing down. handy for comparing with temporally local accelerations
float acc_x_very_smooth = 0;
float acc_y_very_smooth = 0;
float acc_z_very_smooth = 0;

float acc_without_g_x = 0;
float acc_without_g_y = 0;
float acc_without_g_z = 0;
/*
float vel_x = 0;
float vel_y = 0;
float vel_z = 0;
float pos_x = 0;
float pos_y = 0;
float pos_z = 0;
*/


float gyroSensFactor = 0;//factor needed to get to deg/sec
float accelSensFactor = 0;//factor needed to get to m/s

//sens = 0 <-> +- 250 deg/sec
//sens = 1 <-> +- 500 deg/sec
//sens = 2 <-> +- 1000 deg/sec
//sens = 3 <-> +- 2000 deg/sec
void setGyroSensitivity(int sens){
	if(sens < 4 && sens >=0){
		i2c_send(0, 104, 27, 8*sens, 1);
		gyroSensFactor = 250*smallpow(2,sens)/32768.0;
	}else{
		printf("setGyroSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//sens = 0 <-> +- 2g
//sens = 1 <-> +- 4g
//sens = 2 <-> +- 8g
//sens = 3 <-> +- 16g
void setAccelSensitivity(int sens){
	if(sens < 4 && sens >=0){
		i2c_send(0, 104, 28, 8*sens, 1);
		accelSensFactor = 2*9.81*smallpow(2,sens)/32768.0;
	}else{
		printf("setAccelSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//cut off low frequencies using a Digital Low Pass Filter
void enableDLPF(){
	i2c_send(0, 104, 26, 3, 1);
}

void readMPURawData(){
	uint8_t highByte;
	uint8_t lowByte;
	
	//read acc/gyro data at register 59..., 67...
	//GYRO X
	highByte = i2c_receive(0, 104, 67, 1);
	lowByte = i2c_receive(0, 104, 68, 1);
	mpu_pos.gyro_x = gyroSensFactor*(int16_t)((highByte << 8) | lowByte);
	//GYRO Y
	highByte = i2c_receive(0, 104, 69, 1);
	lowByte = i2c_receive(0, 104, 70, 1);
	mpu_pos.gyro_y = gyroSensFactor*(int16_t)((highByte << 8) | lowByte);
	//GYRO Z
	highByte = i2c_receive(0, 104, 71, 1);
	lowByte = i2c_receive(0, 104, 72, 1);
	mpu_pos.gyro_z = gyroSensFactor*(int16_t)((highByte << 8) | lowByte);
	
	//ACCEL X
	highByte = i2c_receive(0, 104, 59, 1);
	lowByte = i2c_receive(0, 104, 60, 1);
	mpu_pos.accel_x = accelSensFactor*(int16_t)((highByte << 8) | lowByte);
	//ACCEL Y
	highByte = i2c_receive(0, 104, 61, 1);
	lowByte = i2c_receive(0, 104, 62, 1);
	mpu_pos.accel_y = accelSensFactor*(int16_t)((highByte << 8) | lowByte);
	//ACCEL Z
	highByte = i2c_receive(0, 104, 63, 1);
	lowByte = i2c_receive(0, 104, 64, 1);
	mpu_pos.accel_z = accelSensFactor*(int16_t)((highByte << 8) | lowByte);
	
	if(CALIBRATION_MODE_MPU6050){
		acc_calibrationx = mpu_pos.accel_x;
		acc_calibrationy = mpu_pos.accel_y;
		acc_calibrationz = mpu_pos.accel_z;
	}
}

// measure delta time (dt) for degrees = Integral(degrees/sec * dt)
int64_t lastMPUtime = 0;
bool firstVisitOfprocessMPURawData = true;
// time difference since last call to processMPURawData in seconds
float time_difference = 0;

// rotation of the drone in world coordinates
float rot[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

//int timeForNormalization = 0;

//TODO: is this ever used? xAxis w.r.t. the PCB, not w.r.t. the plane! (see constants.c Ctrl-F "rot[")
void rotateAroundXAxis(float radians){
	float diff[9];
	diff[0] = 1; //maybe can replace by 1 here
	diff[1] = 0;
	diff[2] = 0;
	
	diff[3] = 0;
	diff[4] = cos(radians);
	diff[5] = -sin(radians);
	
	diff[6] = 0;
	diff[7] = sin(radians);
	diff[8] = cos(radians);
	
	mat_mult(rot, diff);		// writes result to global variable tmp_mat
	array_copy(rot, tmp_mat);
}

void processMPURawData(){
	if(firstVisitOfprocessMPURawData){
		lastMPUtime = esp_timer_get_time();
		firstVisitOfprocessMPURawData = false;
	}else{
		// int64_t esp_timer_get_time() returns time since boot in us (microseconds = 10^-6 seconds)
		int64_t currentTime = esp_timer_get_time();
		time_difference = 0.000001*(float)(currentTime - lastMPUtime);
		lastMPUtime = currentTime;
		
		// matrix based:
		// rotation-matrix:
		// angles in radians
		// 0.01745329 = pi/180
		float alpha = 0.01745329*(mpu_pos.gyro_x-mpu_pos_avg.gyro_x) * time_difference;
		float beta = 0.01745329*(mpu_pos.gyro_y-mpu_pos_avg.gyro_y) * time_difference;
		float gamma = 0.01745329*(mpu_pos.gyro_z-mpu_pos_avg.gyro_z) * time_difference;
		
		// infinitesimal rotation matrix:
		float diff[9];
		diff[0] = cos(beta)*cos(gamma); //maybe can replace by 1 here
		diff[1] = sin(gamma);
		diff[2] = -sin(beta);
		
		diff[3] = -sin(gamma);
		diff[4] = cos(alpha) * cos(gamma);
		diff[5] = -sin(alpha);
		
		diff[6] = sin(beta);
		diff[7] = sin(alpha);
		diff[8] = cos(alpha)*cos(beta);
		
		mat_mult(rot, diff);		// writes result to global variable tmp_mat
		array_copy(rot, tmp_mat);
		
		// normalized current acceleration vector
		float acc_x = mpu_pos.accel_x-accel_offset_x;// zero offset to compensate for constant accelerometer inaccuracies.
		float acc_y = mpu_pos.accel_y-accel_offset_y;// to be found in constants.c
		float acc_z = mpu_pos.accel_z-accel_offset_z;
		
		
		// rotate acc_*_smooth
		mat_transp_mult_vec(diff, acc_x_smooth, acc_y_smooth, acc_z_smooth);
		acc_x_smooth = tmp_vec[0];
		acc_y_smooth = tmp_vec[1];
		acc_z_smooth = tmp_vec[2];
		
		acc_x_smooth = 0.8* acc_x_smooth + 0.2*acc_x;
		acc_y_smooth = 0.8* acc_y_smooth + 0.2*acc_y;
		acc_z_smooth = 0.8* acc_z_smooth + 0.2*acc_z;
		accel_norm_smooth = sqrt((acc_x_smooth*acc_x_smooth)+(acc_y_smooth*acc_y_smooth)+(acc_z_smooth*acc_z_smooth));
		
		
		// rotate acc_*_very_smooth
		mat_transp_mult_vec(diff, acc_x_very_smooth, acc_y_very_smooth, acc_z_very_smooth);
		acc_x_very_smooth = tmp_vec[0];
		acc_y_very_smooth = tmp_vec[1];
		acc_z_very_smooth = tmp_vec[2];
		
		acc_x_very_smooth = 0.999* acc_x_very_smooth + 0.001*acc_x;
		acc_y_very_smooth = 0.999* acc_y_very_smooth + 0.001*acc_y;
		acc_z_very_smooth = 0.999* acc_z_very_smooth + 0.001*acc_z;
		//accel_norm_very_smooth = sqrt((acc_x_very_smooth*acc_x_very_smooth)+(acc_y_very_smooth*acc_y_very_smooth)+(acc_z_very_smooth*acc_z_very_smooth));
		
		// ACCELERATION MINUS GRAVITY in KITE COORDINATES
		acc_without_g_x = acc_x - acc_x_very_smooth;
		acc_without_g_y = acc_y - acc_y_very_smooth;
		acc_without_g_z = acc_z - acc_z_very_smooth;
		// ACCELERATION MINUS GRAVITY from KITE to WORLD COORDINATES
		mat_mult_vec(rot, acc_without_g_x, acc_without_g_y, acc_without_g_z);
		acc_height = tmp_vec[0];
		/*
		// VELOCITY from ACCELERATION
		if(fabs(acc_without_g_x) > 0.12){
			vel_x = vel_x + time_difference*tmp_vec[0];
		}else{
			vel_x *= 0.99;
		}
		if(fabs(acc_without_g_y) > 0.12){
			vel_y = vel_y + time_difference*tmp_vec[1];
		}else{
			vel_y *= 0.99;
		}
		if(fabs(acc_without_g_z) > 0.12){
			vel_z = vel_z + time_difference*tmp_vec[2];
		}else{
			vel_z *= 0.99;
		}
		// POSITION from VELOCITY
		pos_x = pos_x + time_difference*vel_x;
		pos_y = pos_y + time_difference*vel_y;
		pos_z = pos_z + time_difference*vel_z;
		*/
		
		mpu_pos.accel_norm = sqrt((acc_x*acc_x)+(acc_y*acc_y)+(acc_z*acc_z));       //Calculate the total accelerometer vector.
		acc_x /= mpu_pos.accel_norm;
		acc_y /= mpu_pos.accel_norm;
		acc_z /= mpu_pos.accel_norm;
		
		// rotate rotation matrix slightly in the direction where the expected and the currently measured acceleration vectors align.
		rotate_towards_g(rot, mpu_pos_avg.accel_x, mpu_pos_avg.accel_y, mpu_pos_avg.accel_z, acc_x, acc_y, acc_z); // result is written to tmp_mat
		array_copy(rot, tmp_mat);
		
		// how rarely can we normalize without impacting the precision and drift?
		//timeForNormalization ++;
		//if(timeForNormalization > 10){
			normalize_matrix(rot);
		//	timeForNormalization = 0;
		//}
	}
}

void initMPU6050(){
	//wake up MPU6050 from sleep mode
	i2c_send(0, 104, 107, 0, 1);
	
	setGyroSensitivity(1);
	setAccelSensitivity(2);
	enableDLPF();
	
	// measure offset of gyro sensor and
	// initially measured acceleration vector
	mpu_pos_avg.accel_x = 0;
	mpu_pos_avg.accel_y = 0;
	mpu_pos_avg.accel_z = 0;
	mpu_pos_avg.gyro_x = 0;
	mpu_pos_avg.gyro_y = 0;
	mpu_pos_avg.gyro_z = 0;
	
	// averaging for one second
	vTaskDelay(400); // wait for 4 seconds to let the kite come to rest
	for(int i = 0; i < 100; i++){
		readMPURawData();
		mpu_pos_avg.accel_x += mpu_pos.accel_x;
		mpu_pos_avg.accel_y += mpu_pos.accel_y;
		mpu_pos_avg.accel_z += mpu_pos.accel_z;
		mpu_pos_avg.gyro_x += mpu_pos.gyro_x;
		mpu_pos_avg.gyro_y += mpu_pos.gyro_y;
		mpu_pos_avg.gyro_z += mpu_pos.gyro_z;
		vTaskDelay(1);
	}
	mpu_pos_avg.gyro_x *= 0.01;
	mpu_pos_avg.gyro_y *= 0.01;
	mpu_pos_avg.gyro_z *= 0.01;
	
	// those factors are 1/mpu_pos_avg.accel_x when sensor is held such that x is downwards, should move to file containing constants
	mpu_pos_avg.accel_x *= 0.01;
	mpu_pos_avg.accel_x -= accel_offset_x; // accel_offset_x to be found in constants.c
	mpu_pos_avg.accel_y *= 0.01;
	mpu_pos_avg.accel_y -= accel_offset_y;
	mpu_pos_avg.accel_z *= 0.01;
	mpu_pos_avg.accel_z -= accel_offset_z;
	
	
	
	acc_x_smooth = mpu_pos.accel_x-accel_offset_x;
	acc_y_smooth = mpu_pos.accel_y-accel_offset_y;
	acc_z_smooth = mpu_pos.accel_z-accel_offset_z;
	
	
	acc_x_very_smooth = acc_x_smooth;
	acc_y_very_smooth = acc_y_smooth;
	acc_z_very_smooth = acc_z_smooth;
	
	
	// don't worry: factor 0.01 is hidden 5 lines below
	mpu_pos_avg.accel_norm = sqrt((mpu_pos_avg.accel_x*mpu_pos_avg.accel_x)+(mpu_pos_avg.accel_y*mpu_pos_avg.accel_y)+(mpu_pos_avg.accel_z*mpu_pos_avg.accel_z)); //Calculate the norm of the accelerometer vector.
	mpu_pos_avg.accel_x /= mpu_pos_avg.accel_norm;
	mpu_pos_avg.accel_y /= mpu_pos_avg.accel_norm;
	mpu_pos_avg.accel_z /= mpu_pos_avg.accel_norm;
	
	//OVERRIDE EVERYTHING ABOVE TO MAKE Y-AXIS POINT TOWARDS CENTRE OF THE EARTH
	mpu_pos_avg.accel_x = -1;
	mpu_pos_avg.accel_y = 0;
	mpu_pos_avg.accel_z = 0;
}
