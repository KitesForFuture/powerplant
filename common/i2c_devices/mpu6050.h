#ifndef I2C_DEVICES_MPU6050
#define I2C_DEVICES_MPU6050

#include "freertos/FreeRTOS.h"
#include "interchip.h"

struct _Mpu_raw_data {
	float accel[3];
	float gyro[3];
};
typedef struct _Mpu_raw_data Mpu_raw_data;

struct _MPU {
	Mpu_raw_data calibration_data;
	struct i2c_bus bus;
};
typedef struct _MPU MPU;

void initMPU6050(struct i2c_bus bus_arg, Mpu_raw_data calibration_data);

void readMPUData(Mpu_raw_data *out);

#endif
