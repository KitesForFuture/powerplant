#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/mcpwm.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

// some helper functions for mathematical operations
#include "mymath.c"

#include "constants.c"

#include "interchip.c"
#include "cat24c256.c"
// gyroscope and accelerometer
#include "mpu6050.c"

// servo motors and propeller motor control
#include "motors.c"

struct position_data calibration_pos;
void startupMPU6050ForCalibration(){
	//wake up MPU6050 from sleep mode
	i2c_send(0, 104, 107, 0, 1);
	
	setGyroSensitivity(1);
	setAccelSensitivity(2);
	enableDLPF();
}
void calibrateGyro(){
	vTaskDelay(300);
	for(int i = 0; i < 200; i++){
		readMPURawData();
		calibration_pos.gyro_x += mpu_pos.gyro_x;
		calibration_pos.gyro_y += mpu_pos.gyro_y;
		calibration_pos.gyro_z += mpu_pos.gyro_z;
		vTaskDelay(1);
	}
	calibration_pos.gyro_x *= 0.005;
	calibration_pos.gyro_y *= 0.005;
	calibration_pos.gyro_z *= 0.005;
}

void calibrateAccelX(){
	vTaskDelay(100);
	for(int i = 0; i < 100; i++){
		readMPURawData();
		calibration_pos.accel_x += 0.005*mpu_pos.accel_x;
		vTaskDelay(1);
	}
}
void calibrateAccelY(){
	vTaskDelay(100);
	for(int i = 0; i < 100; i++){
		readMPURawData();
		calibration_pos.accel_y += 0.005*mpu_pos.accel_y;
		vTaskDelay(1);
	}
}
void calibrateAccelZ(){
	vTaskDelay(100);
	for(int i = 0; i < 100; i++){
		readMPURawData();
		calibration_pos.accel_z += 0.005*mpu_pos.accel_z;
		vTaskDelay(1);
	}
}


void app_main(void){
	initMotors();
	startupMPU6050ForCalibration();
	
	float servo270Angle = -90;
	float servo180Angle = -90;
	float increment = 0.5;
	int stage = -1;
	int waiter = 0;
	while(1){
		readMPURawData();
		processMPURawData();
		
		if(waiter < 90){
			waiter++;
		}else if(stage == -1){
			calibrateGyro();
			calibrateAccelZ();
			stage = 0;
		}else if(stage == 0){
			servo180Angle += 3*increment;
			if(servo180Angle == 0){
				calibrateAccelX();
				stage = 1;
				waiter = 0;
			}
		}else if(stage == 1){
			servo270Angle += 3*increment;
			if(servo270Angle == -30){
				calibrateAccelY();
				stage = 2;
				waiter = 0;
			}
		}else if(stage == 2){
			servo270Angle += 3*increment;
			if(servo270Angle == 30){
				calibrateAccelX();
				stage = 3;
				waiter = 0;
			}
		}else if(stage == 3){
			servo180Angle += 3*increment;
			if(servo180Angle == 90){
				calibrateAccelZ();
				stage = 4;
				waiter = 0;
			}
		}else if(stage == 4){
			servo180Angle -= 2*increment;
			servo270Angle += increment*4/3;
			if(servo180Angle == 0){
				calibrateAccelY();
				stage = 5;
				waiter = 0;
			}
		}else if(stage == 5){
			servo180Angle -= increment*2;
			servo270Angle -= increment*4;
			if(servo180Angle == -90){
				write2EEPROM(calibration_pos.accel_x, 0);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.accel_y, 1);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.accel_z, 2);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.gyro_x, 3);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.gyro_y, 4);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.gyro_z, 5);
				stage = 7;
				//waiter = 0;
			}
		}
		
		
		
		//printf("stage = %d, waiter = %d, servo180Angle = %f, servo270Angle = %f \n", stage, waiter, servo180Angle, servo270Angle);
		//setAngle(TOP_RIGHT, 30);
		//setAngle(BOTTOM_LEFT, 0);
		setAngle(ELEVON_2, servo270Angle);
		setAngle(ELEVON_1, servo180Angle);
		
	    
	    vTaskDelay(1.0);
	    
    }
    
}
