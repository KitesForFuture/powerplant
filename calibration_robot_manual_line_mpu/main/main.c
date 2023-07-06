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
	i2c_send(0, 105, 107, 0, 1);
	
	setGyroSensitivity(1);
	setAccelSensitivity(2);
	enableDLPF();
}
void calibrateGyro(){
	printf("starting gyro calibration in 3 seconds\n");
	vTaskDelay(300);
	printf("gyro calibrating... (DO NOT MOVE)\n");
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
	printf("acc=(%f, %f, %f), gyr=(%f, %f, %f)\n", readEEPROM(1024+0), readEEPROM(1024+1), readEEPROM(1024+2), readEEPROM(1024+3), readEEPROM(1024+4), readEEPROM(1024+5));
	//initMotors();
	startupMPU6050ForCalibration();
	
	float servo270Angle = -90;
	float servo180Angle = -90;
	float increment = 0.03125;
	int stage = -1;
	int waiter = 0;
	printf("starting calibration soon......\nplace Z pointing UP (SMD pointing UP)\n");
	while(1){
		readMPURawData();
		processMPURawData();
		
		if(waiter < 90){
			waiter++;
		}else if(stage == -1){
			calibrateGyro();
			printf("hold still while calibrating Z...\n");
			calibrateAccelZ();
			stage = 0;
			printf("now place X pointing UP\n");
		}else if(stage == 0){
			servo180Angle += 3*increment;
			if(servo180Angle == 0){
				printf("hold still while calibrating X...\n");
				calibrateAccelX();
				stage = 1;
				waiter = 0;
				printf("now place Y pointing DOWN\n");
			}
		}else if(stage == 1){
			servo270Angle += 3*increment;
			if(servo270Angle == -30){
				printf("hold still while calibrating Y...\n");
				calibrateAccelY();
				stage = 2;
				waiter = 0;
				printf("now place X pointing DOWN\n");
			}
		}else if(stage == 2){
			servo270Angle += 3*increment;
			if(servo270Angle == 30){
				printf("hold still while calibrating X...\n");
				calibrateAccelX();
				stage = 3;
				waiter = 0;
				printf("now place Z pointing DOWN\n");
			}
		}else if(stage == 3){
			servo180Angle += 3*increment;
			if(servo180Angle == 90){
				printf("hold still while calibrating Z...\n");
				calibrateAccelZ();
				stage = 4;
				waiter = 0;
				printf("now place Y pointing UP\n");
			}
		}else if(stage == 4){
			servo180Angle -= 2*increment;
			servo270Angle += increment*4/3;
			if(servo180Angle == 0){
				printf("hold still while calibrating Y...\n");
				calibrateAccelY();
				stage = 5;
				waiter = 0;
				printf("FINISHED CALIBRATION. Writing to EEPROM...\n");
				write2EEPROM(calibration_pos.accel_x, 1024+0);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.accel_y, 1024+1);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.accel_z, 1024+2);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.gyro_x, 1024+3);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.gyro_y, 1024+4);
				vTaskDelay(10.0);
				write2EEPROM(calibration_pos.gyro_z, 1024+5);
				printf("FINISHED WRITING TO EEPROM! YEY.\n");
				stage = 7;
			}
		}
	    vTaskDelay(1.0);
	    
    }
    
}
