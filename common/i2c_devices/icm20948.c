#include "../helpers/math.h"
#include "icm20948.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// HOW TO CALIBRATE:
// output acc_calibrationx,y,z preferably via wifi, set accel_offset_* in constants.c to the midpoints between highest and lowest reading.

//Mpu_raw_data mpu_pos_calibration;
//struct i2c_bus mpu6050_bus;

//float gyro_precision_factor;	//factor needed to get to deg/sec
//float accel_precision_factor;	//factor needed to get to m/s

//sens = 0 <-> +- 250 deg/sec
//sens = 1 <-> +- 500 deg/sec
//sens = 2 <-> +- 1000 deg/sec
//sens = 3 <-> +- 2000 deg/sec
void init_gyro_sensitivityICM20948(ICM20948 *mpu, int sens){
	if(sens < 4 && sens >=0){
		i2c_send(mpu->bus, mpu->address, 127, 2*16, 1); // select User Bank 2
		i2c_send(mpu->bus, mpu->address, 1, 8*3/*DLPF ca. 60Hz*/ + 2*sens + 1/*enable DLPF*/, 1);
		i2c_send(mpu->bus, mpu->address, 127, 0*16, 1); // select User Bank 0
		mpu->gyro_precision_factor = 250*smallpow(2,sens)/32768.0;
	}else{
		printf("setGyroSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//sens = 0 <-> +- 2g
//sens = 1 <-> +- 4g
//sens = 2 <-> +- 8g
//sens = 3 <-> +- 16g
void init_accel_sensitivityICM20948(ICM20948 *mpu, int sens){
	if(sens < 4 && sens >=0){
		i2c_send(mpu->bus, mpu->address, 127, 2*16, 1); // select User Bank 2
		i2c_send(mpu->bus, mpu->address, 20,  8*3/*DLPF ca. 60Hz*/ + 2*sens + 1/*enable DLPF*/, 1);
		i2c_send(mpu->bus, mpu->address, 127, 0*16, 1); // select User Bank 0
		mpu->accel_precision_factor = 2*9.81*smallpow(2,sens)/32768.0;
	}else{
		printf("setAccelSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//cut off low frequencies using a Digital Low Pass Filter
void enableDLPFICM20948(ICM20948 *mpu){
	// merged with init_*_sensitivity
}

void enableMagnetometerICM20948(ICM20948 *mpu){
	i2c_send(mpu->bus, mpu->address, 15, 2, 1); // enable pass-through of slave I2C
	//i2c_send(mpu->bus, mpu->magnetometer_address, 10, 1, 1); // put magnetometer into single measurement mode
}

void readRawDataICM20948(ICM20948 *mpu, raw_data_ICM20948 *out){
	
	//read acc/gyro data at register 59..., 67...
	uint8_t data[12];
	i2c_receiveByteArray(mpu->bus, mpu->address, 45, 12, data);
	
	//ACCEL X
	out->accel[0] = mpu->accel_precision_factor*(int16_t)((data[0] << 8) | data[1]);
	//ACCEL Y
	out->accel[1] = mpu->accel_precision_factor*(int16_t)((data[2] << 8) | data[3]);
	//ACCEL Z
	out->accel[2] = mpu->accel_precision_factor*(int16_t)((data[4] << 8) | data[5]);
	
	//GYRO X
	out->gyro[0] = mpu->gyro_precision_factor*(int16_t)((data[6] << 8) | data[7]);
	//GYRO Y
	out->gyro[1] = mpu->gyro_precision_factor*(int16_t)((data[8] << 8) | data[9]);
	//GYRO Z
	out->gyro[2] = mpu->gyro_precision_factor*(int16_t)((data[10] << 8) | data[11]);
	
	
	//MAGNET
	uint8_t ST1;
	
	ST1 = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x10, 1);
	if(ST1 & 0x01){
		
		i2c_receiveByteArray(mpu->bus, mpu->magnetometer_address, 0x11, 6, data);
		out->magnet[1] = 1.0f*(int16_t)((data[1] << 8) | data[0]);
		
		out->magnet[0] = 1.0f*(int16_t)((data[3] << 8) | data[2]);
		
		out->magnet[2] = 1.0f*(int16_t)((data[5] << 8) | data[4]);
		
		i2c_send(mpu->bus, mpu->magnetometer_address, 0x31, 1, 1);//go into single measurement mode 
	}
	
}

void readDataICM20948(ICM20948 *mpu, raw_data_ICM20948 *position){
	readRawDataICM20948(mpu, position);
	printf("%f, %f, %f\n", position->accel[0], position->accel[1], position->accel[2]);
	position->accel[0] -= mpu->calibration_data.accel[0];
	position->accel[1] -= mpu->calibration_data.accel[1];
	position->accel[2] -= mpu->calibration_data.accel[2];
	position->gyro[0] -= mpu->calibration_data.gyro[0];
	position->gyro[1] -= mpu->calibration_data.gyro[1];
	position->gyro[2] -= mpu->calibration_data.gyro[2];
	position->magnet[0] -= mpu->calibration_data.magnet[0];
	position->magnet[1] -= mpu->calibration_data.magnet[1];
	position->magnet[2] -= mpu->calibration_data.magnet[2];
}

void initICM20948(ICM20948 *mpu){//struct i2c_bus bus_arg, Mpu_raw_data calibration_data){

	//wake up ICM20948 from sleep mode
	vTaskDelay(10);
	i2c_send(mpu->bus, mpu->address, 127, 0, 1);
	vTaskDelay(10);
	i2c_send(mpu->bus, mpu->address, 6, 1, 1);
	vTaskDelay(10);
	printf("%d\n", i2c_receive(mpu->bus, mpu->address, 45, 1));

	//mpu_pos_calibration = calibration_data;
	
	init_gyro_sensitivityICM20948(mpu, 1);
	init_accel_sensitivityICM20948(mpu, 2);
	
	enableMagnetometerICM20948(mpu);
	i2c_send(mpu->bus, mpu->magnetometer_address, 0x31, 1, 1);//go into single measurement mode
}
