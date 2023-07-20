#include "../helpers/math.h"
#include "mpu9250.h"


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
void init_gyro_sensitivity9250(MPU9250 *mpu, int sens){
	if(sens < 4 && sens >=0){
		i2c_send(mpu->bus, mpu->address, 27, 8*sens, 1);
		mpu->gyro_precision_factor = 250*smallpow(2,sens)/32768.0;
	}else{
		printf("setGyroSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//sens = 0 <-> +- 2g
//sens = 1 <-> +- 4g
//sens = 2 <-> +- 8g
//sens = 3 <-> +- 16g
void init_accel_sensitivity9250(MPU9250 *mpu, int sens){
	if(sens < 4 && sens >=0){
		i2c_send(mpu->bus, mpu->address, 28, 8*sens, 1);
		mpu->accel_precision_factor = 2*9.81*smallpow(2,sens)/32768.0;
	}else{
		printf("setAccelSensitivity(int sens), sensitivity must be between 0 and 3");
	}
}

//cut off low frequencies using a Digital Low Pass Filter
void enableDLPF9250(MPU9250 *mpu){
	i2c_send(mpu->bus, mpu->address, 26, 3, 1);
}

void enableMagnetometer(MPU9250 *mpu){
	i2c_send(mpu->bus, mpu->address, 55, 2, 1); // enable pass-through of slave I2C
	//i2c_send(mpu->bus, mpu->magnetometer_address, 10, 1, 1); // put magnetometer into single measurement mode
}

void readMPURawData9250(MPU9250 *mpu, Mpu_raw_data_9250 *out){
	uint8_t highByte;
	uint8_t lowByte;
	
	//read acc/gyro data at register 59..., 67...
	//GYRO X
	highByte = i2c_receive(mpu->bus, mpu->address, 67, 1);
	lowByte = i2c_receive(mpu->bus, mpu->address, 68, 1);
	out->gyro[0] = mpu->gyro_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//GYRO Y
	highByte = i2c_receive(mpu->bus, mpu->address, 69, 1);
	lowByte = i2c_receive(mpu->bus, mpu->address, 70, 1);
	out->gyro[1] = mpu->gyro_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//GYRO Z
	highByte = i2c_receive(mpu->bus, mpu->address, 71, 1);
	lowByte = i2c_receive(mpu->bus, mpu->address, 72, 1);
	out->gyro[2] = mpu->gyro_precision_factor*(int16_t)((highByte << 8) | lowByte);
	
  
	//ACCEL X
	highByte = i2c_receive(mpu->bus, mpu->address, 59, 1);
	lowByte = i2c_receive(mpu->bus, mpu->address, 60, 1);
	out->accel[0] = mpu->accel_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//ACCEL Y
	highByte = i2c_receive(mpu->bus, mpu->address, 61, 1);
	lowByte = i2c_receive(mpu->bus, mpu->address, 62, 1);
	out->accel[1] = mpu->accel_precision_factor*(int16_t)((highByte << 8) | lowByte);
	//ACCEL Z
	highByte = i2c_receive(mpu->bus, mpu->address, 63, 1);
	lowByte = i2c_receive(mpu->bus, mpu->address, 64, 1);
	out->accel[2] = mpu->accel_precision_factor*(int16_t)((highByte << 8) | lowByte);
	
	
	//MAGNET
	
	uint8_t ST1;
	i2c_send(mpu->bus, mpu->magnetometer_address, 10, 1, 1);//go into single measurement mode 
	do
	{
		ST1 = i2c_receive(mpu->bus, mpu->magnetometer_address, 2, 1);
		//printf("waiting, ST1 = %d\n", ST1);
	} while (!(ST1 & 0x01));
	
	lowByte = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x03, 1);
	highByte = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x04, 1);
	out->magnet[1] = 1.0f*(int16_t)((highByte << 8) | lowByte);
	
	lowByte = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x05, 1);
	highByte = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x06, 1);
	out->magnet[0] = 1.0f*(int16_t)((highByte << 8) | lowByte);
	
	lowByte = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x07, 1);
	highByte = i2c_receive(mpu->bus, mpu->magnetometer_address, 0x08, 1);
	out->magnet[2] = -1.0f*(int16_t)((highByte << 8) | lowByte);
	
	//printf("%f, %f, %f\n", out->magnet[0], out->magnet[1], out->magnet[2]);
	
	/*
	uint8_t ST1;
	I2CwriteByte(MAG_ADDRESS,0x0A,0x01);
	do
	{
	I2Cread(MAG_ADDRESS, 0x02, 1, &ST1);
	} while (!(ST1 & 0x01));

	uint8_t Mag[7];
	I2Cread(MAG_ADDRESS, 0x03, 7, Mag);

	// Convertir registros magnetometro
	int16_t mx = -(Mag[3] << 8 | Mag[2]);
	int16_t my = -(Mag[1] << 8 | Mag[0]);
	int16_t mz = -(Mag[5] << 8 | Mag[4]);
	*/
	
}

void readMPUData9250(MPU9250 *mpu, Mpu_raw_data_9250 *position){
	readMPURawData9250(mpu, position);
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

void initMPU9250(MPU9250 *mpu){//struct i2c_bus bus_arg, Mpu_raw_data calibration_data){
	
  	//mpu6050_bus = bus_arg;

	//wake up MPU6050 from sleep mode
	i2c_send(mpu->bus, mpu->address, 107, 0, 1);

	//mpu_pos_calibration = calibration_data;
	
	init_gyro_sensitivity9250(mpu, 1);
	init_accel_sensitivity9250(mpu, 2);
	enableDLPF9250(mpu);
	enableMagnetometer(mpu);
}
