#include "rotation_matrix.h"

// COORDINATE SYSTEM OF MPU (in vector subtraction notation):
// X-Axis: GYRO chip - FUTURE silk writing
// Y-Axis: BMP chip - BATTERY CONNECTORS
// Z-Axis: custom board - ESP32 board

// COORDINATE SYSTEM OF KITE (in vector subtraction notation):
// X-Axis: head - tail
// Y-Axis: left wing - right wing
// Z-Axis: kite - ground station

//KITE COORDINATE AXES EXPRESSED IN TERMS OF MPU COORDINATE AXES
//SURVIVOR:
/*
#define accel_x -mpu_raw_data.accel[1]
#define accel_y -mpu_raw_data.accel[0]
#define accel_z -mpu_raw_data.accel[2]

#define gyro_x -mpu_raw_data.gyro[1]
#define gyro_y -mpu_raw_data.gyro[0]
#define gyro_z -mpu_raw_data.gyro[2]	
*/

//BEBUEGELTER und neuer Autopilot neutral:
/*
#define accel_x mpu_raw_data.accel[0]
#define accel_y mpu_raw_data.accel[1]
#define accel_z mpu_raw_data.accel[2]

#define gyro_x mpu_raw_data.gyro[0]
#define gyro_y mpu_raw_data.gyro[1]
#define gyro_z mpu_raw_data.gyro[2]
*/
//Neuer Autopilot, seitlich eingebaut, Akku-Anschluss rechts, ESP32 richtung Bauch
/*
#define accel_x mpu_raw_data.accel[1]
#define accel_y -mpu_raw_data.accel[0]
#define accel_z mpu_raw_data.accel[2]

#define gyro_x mpu_raw_data.gyro[1]
#define gyro_y -mpu_raw_data.gyro[0]
#define gyro_z mpu_raw_data.gyro[2]
*/

//Erster EPP-Flügel, längs eingebaut, Akku-Anschluss hinten, ESP32 richtung Bauch

#define accel_x -mpu_raw_data.accel[0]
#define accel_y -mpu_raw_data.accel[1]
#define accel_z mpu_raw_data.accel[2]

#define gyro_x -mpu_raw_data.gyro[0]
#define gyro_y -mpu_raw_data.gyro[1]
#define gyro_z mpu_raw_data.gyro[2]

// The Gravity vector is the direction the gravitational force is supposed to point in KITE COORDINATES with the nose pointing to the sky
#define gravity_x 1
#define gravity_y 0
#define gravity_z 0

// rotation of the drone in world coordinates
//float rotation_matrix[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
//float gyro_in_kite_coords[3] = {0,0,0};

//void sendData(float data0, float data1, float data2, float data3, float data4, float data5, float data6, float data7, float data8, float data9, float data10, float data11, float data12, float data13, float data14, float data15, float data16, float data17, float data18, float data19, float data20, float data21, float data22);

Mpu_raw_data mpu_raw_data = {
	{0, 0, 0},
	{0, 0, 0}
};

Time mpu_last_update_time = 0;

float getAccelX(){
	return accel_x;
}

void initRotationMatrix(Orientation_Data* orientation_data){
	float tmp[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};
	memcpy(orientation_data->rotation_matrix, tmp, 9*sizeof(float));
	float tmp_gyro[3] = {0,0,0};
	memcpy(orientation_data->gyro_in_kite_coords, tmp_gyro, 3*sizeof(float));
}

void FAKEupdateRotationMatrix(Orientation_Data* orientation_data){
	
	if(mpu_last_update_time == 0){
		mpu_last_update_time = start_timer();
		return;
	}
	float time_difference = query_timer_seconds(mpu_last_update_time);
	mpu_last_update_time = start_timer();
	
	// ROTATE 20 deg/s AROUND EVERY AXIS
	orientation_data->gyro_in_kite_coords[0] = 20 * PI / 180;
	orientation_data->gyro_in_kite_coords[1] = 20 * PI / 180;
	orientation_data->gyro_in_kite_coords[2] = 20 * PI / 180;
	
	float alpha = PI / 180 * 20 * time_difference;
	float beta = PI / 180 * 20 * time_difference;
	float gamma = PI / 180 * 20 * time_difference;
	
	// infinitesimal rotation matrix:
	float diff[9];
	diff[0] = 1;
	diff[1] = -gamma;
	diff[2] = beta;
	
	diff[3] = gamma;
	diff[4] = 1;
	diff[5] = -alpha;
	
	diff[6] = -beta;
	diff[7] = alpha;
	diff[8] = 1;
	
	float temp_rotation_matrix[9];
	mat_mult(orientation_data->rotation_matrix, diff, temp_rotation_matrix);
	memcpy(orientation_data->rotation_matrix, temp_rotation_matrix, sizeof(temp_rotation_matrix));
	normalize_matrix(orientation_data->rotation_matrix);
	
	orientation_data->rotation_matrix_transpose[0] = orientation_data->rotation_matrix[0];
	orientation_data->rotation_matrix_transpose[1] = orientation_data->rotation_matrix[3];
	orientation_data->rotation_matrix_transpose[2] = orientation_data->rotation_matrix[6];
	orientation_data->rotation_matrix_transpose[3] = orientation_data->rotation_matrix[1];
	orientation_data->rotation_matrix_transpose[4] = orientation_data->rotation_matrix[4];
	orientation_data->rotation_matrix_transpose[5] = orientation_data->rotation_matrix[7];
	orientation_data->rotation_matrix_transpose[6] = orientation_data->rotation_matrix[2];
	orientation_data->rotation_matrix_transpose[7] = orientation_data->rotation_matrix[5];
	orientation_data->rotation_matrix_transpose[8] = orientation_data->rotation_matrix[8];
}

void updateRotationMatrix(Orientation_Data* orientation_data){
	readMPUData(&mpu_raw_data);
	if(mpu_last_update_time == 0){
		mpu_last_update_time = start_timer();
		return;
	}
	float time_difference = query_timer_seconds(mpu_last_update_time);
	mpu_last_update_time = start_timer();
	
	// matrix based:
	// rotation-matrix:
	// angles in radians
	// 0.01745329 = pi/180
	
	orientation_data->gyro_in_kite_coords[0] = gyro_x * PI / 180;
	orientation_data->gyro_in_kite_coords[1] = gyro_y * PI / 180;
	orientation_data->gyro_in_kite_coords[2] = gyro_z * PI / 180;
	
	float alpha = PI / 180 * gyro_x * time_difference;
	float beta = PI / 180 * gyro_y * time_difference;
	float gamma = PI / 180 * gyro_z * time_difference;
	
	//sendData(alpha, beta, gamma, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	
	// infinitesimal rotation matrix:
	float diff[9];
	diff[0] = 1; //maybe can replace by 1 here
	diff[1] = -gamma;
	diff[2] = beta;
	
	diff[3] = gamma;
	diff[4] = 1;
	diff[5] = -alpha;
	
	diff[6] = -beta;
	diff[7] = alpha;
	diff[8] = 1;
	
	float temp_rotation_matrix[9];
	mat_mult(orientation_data->rotation_matrix, diff, temp_rotation_matrix);
	
	rotate_towards_g(temp_rotation_matrix, gravity_x, gravity_y, gravity_z, accel_x, accel_y, accel_z, orientation_data->rotation_matrix);
	//memcpy(orientation_data->rotation_matrix, temp_rotation_matrix, sizeof(temp_rotation_matrix));// TODO: remove when above line uncommented!!!
	
	normalize_matrix(orientation_data->rotation_matrix);
	
	orientation_data->rotation_matrix_transpose[0] = orientation_data->rotation_matrix[0];
	orientation_data->rotation_matrix_transpose[1] = orientation_data->rotation_matrix[3];
	orientation_data->rotation_matrix_transpose[2] = orientation_data->rotation_matrix[6];
	orientation_data->rotation_matrix_transpose[3] = orientation_data->rotation_matrix[1];
	orientation_data->rotation_matrix_transpose[4] = orientation_data->rotation_matrix[4];
	orientation_data->rotation_matrix_transpose[5] = orientation_data->rotation_matrix[7];
	orientation_data->rotation_matrix_transpose[6] = orientation_data->rotation_matrix[2];
	orientation_data->rotation_matrix_transpose[7] = orientation_data->rotation_matrix[5];
	orientation_data->rotation_matrix_transpose[8] = orientation_data->rotation_matrix[8];
}


