#include <math.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

//#include "driver/mcpwm.h"

#include "driver/gpio.h"
#include "driver/i2c.h"

// some helper functions for mathematical operations
#include "mymath.c"

//#include "constants.c"

#include "../../common/i2c_devices/cat24c256.h"
#include "../../common/i2c_devices/icm20948.h"

//#include "interchip.c"
//#include "cat24c256.c"
// gyroscope and accelerometer
//#include "mpu6050.c"

// servo motors and propeller motor control
//#include "motors.c"

struct i2c_bus bus0 = {18, 19};
struct i2c_bus bus1 = {25, 14};

struct position_data{
	float accel_norm;
	
	float accel_x;
	float accel_y;
	float accel_z;
	
	float gyro_x;
	float gyro_y;
	float gyro_z;
};

struct position_data calibration_pos;
raw_data_ICM20948 kite_and_line_mpu_calibration = {
		{0.0, 0.0, 0.0}, // accel
		{0.0, 0.0, 0.0}, // gyro
		{0.0, 0.0, 0.0} // magnet
	};;
raw_data_ICM20948 kite_and_line_mpu_raw_data = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};;
ICM20948 kite_and_line_mpu;

void calibrateGyro(){
	printf("starting gyro calibration in 3 seconds\n");
	vTaskDelay(300);
	printf("gyro calibrating... (DO NOT MOVE)\n");
	for(int i = 0; i < 200; i++){
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		calibration_pos.gyro_x += kite_and_line_mpu_raw_data.gyro[0];
		calibration_pos.gyro_y += kite_and_line_mpu_raw_data.gyro[1];
		calibration_pos.gyro_z += kite_and_line_mpu_raw_data.gyro[2];
		vTaskDelay(1);
	}
	calibration_pos.gyro_x *= 0.005;
	calibration_pos.gyro_y *= 0.005;
	calibration_pos.gyro_z *= 0.005;
}

void calibrateAccelX(){
	vTaskDelay(100);
	for(int i = 0; i < 100; i++){
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		calibration_pos.accel_x += 0.005*kite_and_line_mpu_raw_data.accel[0];
		vTaskDelay(1);
	}
}
void calibrateAccelY(){
	vTaskDelay(100);
	for(int i = 0; i < 100; i++){
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		calibration_pos.accel_y += 0.005*kite_and_line_mpu_raw_data.accel[1];
		vTaskDelay(1);
	}
}
void calibrateAccelZ(){
	vTaskDelay(100);
	for(int i = 0; i < 100; i++){
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		calibration_pos.accel_z += 0.005*kite_and_line_mpu_raw_data.accel[2];
		vTaskDelay(1);
	}
}

void app_main(void){
	
	init_cat24(bus1);
	
	printf("acc=(%f, %f, %f), gyr=(%f, %f, %f)\n", readEEPROM(0), readEEPROM(1), readEEPROM(2), readEEPROM(3), readEEPROM(4), readEEPROM(5));
	//initMotors();
	
	kite_and_line_mpu.bus = bus0;
	kite_and_line_mpu.address = 0x68;
	kite_and_line_mpu.magnetometer_address = 12;
	kite_and_line_mpu.calibration_data = kite_and_line_mpu_calibration;
    initICM20948(&kite_and_line_mpu);
    printf("initialized icm-20948");
	
	
	readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
	
	
	float servo270Angle = -90;
	float servo180Angle = -90;
	float increment = 0.03125;
	int stage = -1;
	int waiter = 0;
	printf("starting calibration soon......\nplace Z pointing UP (SMD pointing UP)\n");
	while(1){
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		//processMPURawData();
		
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
				printf("FINISHED WRITING TO EEPROM! YEY.\n");
				stage = 7;
			}
		}
	    vTaskDelay(1.0);
	    
    }
    
}
