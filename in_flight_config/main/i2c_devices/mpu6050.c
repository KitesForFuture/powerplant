

#include "../helpers/math.h"
#include "mpu6050.h"


// HOW TO CALIBRATE:
// output acc_calibrationx,y,z preferably via wifi, set accel_offset_* in constants.c to the midpoints between highest and lowest reading.

Mpu_raw_data mpu_pos_calibration;
struct i2c_bus mpu6050_bus;

float gyro_precision_factor;	//factor needed to get to deg/sec
float accel_precision_factor;	//factor needed to get to m/s

//sens = 0 <-> +- 250 deg/sec
//sens = 1 <-> +- 500 deg/sec
//sens = 2 <-> +- 1000 deg/sec
//sens = 3 <-> +- 2000 deg/sec
void init_gyro_sensitivity(int sens){
	if(sens < 4 && sens >=0){
		i2c_send(mpu6050_bus, 104, 27, 8*sens, 1);
		gyro_precision_factor = 250*smallpow(2,sens)/32768.0;
	}else{
		printf("setGyroSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//sens = 0 <-> +- 2g
//sens = 1 <-> +- 4g
//sens = 2 <-> +- 8g
//sens = 3 <-> +- 16g
void init_accel_sensitivity(int sens){
	if(sens < 4 && sens >=0){
		i2c_send(mpu6050_bus, 104, 28, 8*sens, 1);
		accel_precision_factor = 2*9.81*smallpow(2,sens)/32768.0;
	}else{
		printf("setAccelSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//cut off low frequencies using a Digital Low Pass Filter
void enableDLPF(){
	i2c_send(mpu6050_bus, 104, 26, 3, 1);
}

void readMPURawData(Mpu_raw_data *out){
	uint8_t highByte;
	uint8_t lowByte;
	
	//read acc/gyro data at register 59..., 67...
	//GYRO X
	highByte = i2c_receive(mpu6050_bus, 104, 67, 1);
	lowByte = i2c_receive(mpu6050_bus, 104, 68, 1);
	out->gyro[0] = gyro_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//GYRO Y
	highByte = i2c_receive(mpu6050_bus, 104, 69, 1);
	lowByte = i2c_receive(mpu6050_bus, 104, 70, 1);
	out->gyro[1] = gyro_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//GYRO Z
	highByte = i2c_receive(mpu6050_bus, 104, 71, 1);
	lowByte = i2c_receive(mpu6050_bus, 104, 72, 1);
	out->gyro[2] = gyro_precision_factor*(int16_t)((highByte << 8) | lowByte);
	
  
	//ACCEL X
	highByte = i2c_receive(mpu6050_bus, 104, 59, 1);
	lowByte = i2c_receive(mpu6050_bus, 104, 60, 1);
	out->accel[0] = accel_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//ACCEL Y
	highByte = i2c_receive(mpu6050_bus, 104, 61, 1);
	lowByte = i2c_receive(mpu6050_bus, 104, 62, 1);
	out->accel[1] = accel_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//ACCEL Z
	highByte = i2c_receive(mpu6050_bus, 104, 63, 1);
	lowByte = i2c_receive(mpu6050_bus, 104, 64, 1);
	out->accel[2] = accel_precision_factor*(int16_t)((highByte << 8) | lowByte);
}

void readMPUData(Mpu_raw_data *position){
	readMPURawData(position);
	position->accel[0] -= mpu_pos_calibration.accel[0];
	position->accel[1] -= mpu_pos_calibration.accel[1];
	position->accel[2] -= mpu_pos_calibration.accel[2];
	position->gyro[0] -= mpu_pos_calibration.gyro[0];
	position->gyro[1] -= mpu_pos_calibration.gyro[1];
	position->gyro[2] -= mpu_pos_calibration.gyro[2];
}

void initMPU6050(struct i2c_bus bus_arg, Mpu_raw_data calibration_data){
	
  	mpu6050_bus = bus_arg;

	//wake up MPU6050 from sleep mode
	i2c_send(mpu6050_bus, 104, 107, 0, 1);

	mpu_pos_calibration = calibration_data;
	
	init_gyro_sensitivity(1);
	init_accel_sensitivity(2);
	enableDLPF();
}
