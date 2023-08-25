#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


#include "../../common/i2c_devices/cat24c256.h"
//#include "../../common/i2c_devices/mpu6050.h"
#include "../../common/i2c_devices/mpu9250.h"

#include "control/rotation_matrix.h"
#include "../../common/pwm/motors.h"
//#include "../../common/pwm/pwm_input.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "../../common/RC.c"

// for Access Point
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"
// for http Web Server
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>

#include "RC_for_config.c"

#include "control/autopilot.h"

#define MAX_SERVO_DEFLECTION 80
#define MAX_BRAKE_DEFLECTION 80
#define MAX_PROPELLER_SPEED 90 // AT MOST 90

struct i2c_bus bus0 = {14, 25};
struct i2c_bus bus1 = {18, 19};

static Autopilot autopilot;

float config_values[NUM_CONFIG_FLOAT_VARS];
int data_needs_being_written_to_EEPROM = 0;

void writeConfigValuesToEEPROM(float* values){
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
		float readValue = readEEPROM(i);
		if(readValue != values[i]) write2EEPROM(values[i], i);
	}
	printf("values[6] to write = %f\n", values[6]);
}

void readConfigValuesFromEEPROM(float* values){
	for (int i = 0; i < NUM_CONFIG_FLOAT_VARS; i++){
		values[i] = readEEPROM(i);
	}
}

void getConfigValues(float* values){
	for (int i = 0; i < NUM_CONFIG_FLOAT_VARS; i++){
		values[i] = config_values[i];
	}
}
int groundstation_has_config_values_initialized_from_kite_EEPROM = false;
void setConfigValues(float* values){
	if(values[6] == 0) printf("here bmp_calib is 0\n");
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
		config_values[i] = values[i];
	}
	//updateBMP280Config(config_values[6]);
	data_needs_being_written_to_EEPROM = 1;
	groundstation_has_config_values_initialized_from_kite_EEPROM = true;
	loadConfigVariables(&autopilot, config_values);
}

void actuatorControl(float left_elevon, float right_elevon, float brake, float rudder, float left_propeller, float right_propeller, float propeller_safety_max){
	
	if(config_values[9]){ // SWAPPED
		setAngle(3, config_values[37] + config_values[7]*left_elevon); // left elevon
		setAngle(0, config_values[38] + config_values[8]*right_elevon); // right elevon
	}else{
		setAngle(0, config_values[37] + config_values[7]*left_elevon); // left elevon
		setAngle(3, config_values[38] + config_values[8]*right_elevon); // right elevon
	}
	
	if(config_values[11]){ // SWAPPED
		setSpeed(2, clamp(left_propeller, 0, propeller_safety_max)); // left Propeller
		setSpeed(4, clamp(right_propeller, 0, propeller_safety_max)); // right Propeller
	}else{
		setSpeed(4, clamp(left_propeller, 0, propeller_safety_max)); // left Propeller
		setSpeed(2, clamp(right_propeller, 0, propeller_safety_max)); // right Propeller
	}
	setAngle(1, config_values[39] + config_values[10]*brake); // Brake
	setAngle(5, config_values[41] + config_values[40]*rudder); // Rudder
}

// can also be used to manually change config variables
void testConfigWriting(){
	readConfigValuesFromEEPROM(config_values);
	
	float test_config[NUM_CONFIG_FLOAT_VARS];
	getConfigValues(test_config);
	printf("config[6]*1000000000 = %f\n", (test_config[6]*1000000000));
	test_config[6] += 0.00000123;
	printf("after adding 0.00000123, config[6]*1000000000 = %f\n", (test_config[6]*1000000000));
	
	setConfigValues(test_config);
	vTaskDelay(100);
	getConfigValues(test_config);
	printf("after writing to and reading from EEPROM, config[6]*1000000000 = %f\n", (test_config[6]*1000000000));
}

void main_task(void* arg)
{
	init_uptime();
	
	Orientation_Data kite_orientation_data;
	initRotationMatrix(&kite_orientation_data);
	
	Orientation_Data line_orientation_data;
	initRotationMatrix(&line_orientation_data);
	
	
	init_cat24(bus1);
	
	//testConfigWriting();//TODO: remove. DEBUGGING ONLY
	
	/*Mpu_raw_data kite_mpu_calibration = {
		{readEEPROM(0), readEEPROM(1), readEEPROM(2)},
		{readEEPROM(3), readEEPROM(4), readEEPROM(5)}
	};*/
	
	Mpu_raw_data_9250 line_mpu_calibration = {
		{0.132, 0.14, 0.135},
		{1.74, 0.93, 0.08},
		{65.6, 6.8, 22.5}
	};
	
	int output_pins[] = {27,26,12,13,5,15};
	initMotors(output_pins, 6);
	
	setAngle(0, 0);
	setAngle(1, 0);
	setAngle(3, 0);
	setAngle(5, 0);
	setSpeed(2, 0);
	setSpeed(4, 0);
	//setSpeed(2, 90);
	//setSpeed(4, 90);
	//MPU kite_mpu;
	MPU9250 line_mpu;
	//kite_mpu.bus = bus0;
	line_mpu.bus = bus0;
	//kite_mpu.address = 104;
	line_mpu.address = 105;
	line_mpu.magnetometer_address = 12;
	//kite_mpu.calibration_data = kite_mpu_calibration;
	line_mpu.calibration_data = line_mpu_calibration;
    //initMPU6050(&kite_mpu);
    initMPU9250(&line_mpu);
    /*Mpu_raw_data kite_mpu_raw_data = {
		{0, 0, 0},
		{0, 0, 0}
	};*/
	Mpu_raw_data_9250 line_mpu_raw_data = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};
	
	readConfigValuesFromEEPROM(config_values);
	network_setup_configuring(&getConfigValues ,&setConfigValues, &actuatorControl, &kite_orientation_data, &line_orientation_data);
	
	//float mag_x[128];
	//float mag_y[128];
	//float mag_z[128];
	//int magnet_calibration_data_counter = 0;
	
	int counter = 0;
	int avg = 256;
	int calibration = false;
	float gyro_x = 0;
	float gyro_y = 0;
	float gyro_z = 0;
	float accel_x = 0;
	float accel_y = 0;
	float accel_z = 0;
	float mag_x = 0;
	float mag_y = 0;
	float mag_z = 0;
	while(1) {
		vTaskDelay(1);
		
		if(calibration){
			
			readMPURawData9250(&line_mpu, &line_mpu_raw_data);
			if(counter == avg){
				printf("gyro = %f, %f, %f, accel = %f, %f, %f, mag = %f, %f, %f\n", gyro_x/avg, gyro_y/avg, gyro_z/avg, accel_x/avg, accel_y/avg, accel_z/avg, mag_x/avg, mag_y/avg, mag_z/avg);
				counter = 0;
				gyro_x = 0;
				gyro_y = 0;
				gyro_z = 0;
				accel_x = 0;
				accel_y = 0;
				accel_z = 0;
				mag_x = 0;
				mag_y = 0;
				mag_z = 0;
			}else{
				gyro_x += line_mpu_raw_data.gyro[0];
				gyro_y += line_mpu_raw_data.gyro[1];
				gyro_z += line_mpu_raw_data.gyro[2];
				accel_x += line_mpu_raw_data.accel[0];
				accel_y += line_mpu_raw_data.accel[1];
				accel_z += line_mpu_raw_data.accel[2];
				mag_x += line_mpu_raw_data.magnet[0];
				mag_y += line_mpu_raw_data.magnet[1];
				mag_z += line_mpu_raw_data.magnet[2];
				counter ++;
			}
			continue;
		}
		
		//readMPUData(&kite_mpu, &kite_mpu_raw_data);
		readMPUData9250(&line_mpu, &line_mpu_raw_data);
		/*printf("accel = %f, %f, %f, gyro = %f, %f, %f, magnet = %f, %f, %f\n",
		line_mpu_raw_data.accel[0], line_mpu_raw_data.accel[1], line_mpu_raw_data.accel[2],
		line_mpu_raw_data.gyro[0], line_mpu_raw_data.gyro[1], line_mpu_raw_data.gyro[2],
		line_mpu_raw_data.magnet[0], line_mpu_raw_data.magnet[1], line_mpu_raw_data.magnet[2]);
		*/
		updateRotationMatrix(&line_orientation_data, line_mpu_raw_data);
		float* mat = line_orientation_data.rotation_matrix;
		
		/*
		printf("rotation_matrix = %f, %f, %f, %f, %f, %f, %f, %f, %f\n",
		mat[0], mat[1], mat[2],
		mat[3], mat[4], mat[5],
		mat[6], mat[7], mat[8]);
		*/
		
		/*
		if(magnet_calibration_data_counter < 128){
			mag_x[magnet_calibration_data_counter] = line_mpu_raw_data.magnet[0];
			mag_y[magnet_calibration_data_counter] = line_mpu_raw_data.magnet[1];
			mag_z[magnet_calibration_data_counter] = line_mpu_raw_data.magnet[2];
			magnet_calibration_data_counter++;
			printf("%d\n", magnet_calibration_data_counter);
		}else if (magnet_calibration_data_counter == 128){
			
			float a_x = 0;
			float b_x = 1;
			float a_y = 0;
			float b_y = 1;
			float a_z = 0;
			float b_z = 1;
			
			for(int j = 0; j < 10000; j++){
				vTaskDelay(1);
				int random = rand() % 128;
				float x = mag_x[random];
				float y = mag_y[random];
				float z = mag_z[random];
				float delta_norm = 50*50 - ((a_x + b_x * x)*(a_x + b_x * x) + (a_y + b_y * y)*(a_y + b_y * y) + (a_z + b_z * z)*(a_z + b_z * z));
				printf("norm = %f\n", delta_norm);
				printf("a_x,a_y,a_z = %f, %f, %f, b_x,b_y,b_z = %f, %f, %f\n", a_x, a_y, a_z, b_x, b_y, b_z);
				//printf("b_x,b_y,b_z = %f, %f, %f\n", b_x, b_y, b_z);
				for(int i = 0; i < 20000; i++){
					
					int random = rand() % 128;
					
					x = mag_x[random];
					y = mag_y[random];
					z = mag_z[random];
					//printf("x,y,z = %f, %f, %f, rand = %d\n", x, y, z, random);
					//printf("a_x,a_y,a_z = %f, %f, %f\n", a_x, a_y, a_z);
					//printf("b_x,b_y,b_z = %f, %f, %f\n", b_x, b_y, b_z);
					float delta_norm = 50*50 - ((a_x + b_x * x)*(a_x + b_x * x) + (a_y + b_y * y)*(a_y + b_y * y) + (a_z + b_z * z)*(a_z + b_z * z));
					//printf("norm = %f\n", delta_norm);
					
					
					// GRADIENT
					float delta_f_delta_a_x = -2*(a_x + b_x * x);
					float delta_f_delta_a_y = -2*(a_y + b_y * y);
					float delta_f_delta_a_z = -2*(a_z + b_z * z);
					
					float delta_f_delta_b_x = x*delta_f_delta_a_x;
					float delta_f_delta_b_y = y*delta_f_delta_a_y;
					float delta_f_delta_b_z = z*delta_f_delta_a_z;
					
					float h = 0.01 * delta_norm / (delta_f_delta_a_x*delta_f_delta_a_x + delta_f_delta_a_y*delta_f_delta_a_y + delta_f_delta_a_z*delta_f_delta_a_z
					+ delta_f_delta_b_x*delta_f_delta_b_x + delta_f_delta_b_y*delta_f_delta_b_y + delta_f_delta_b_z*delta_f_delta_b_z);
					
					//printf("gradient = %f, %f, %f, %f, %f, %f\n", delta_f_delta_a_x, delta_f_delta_a_y, delta_f_delta_a_z, delta_f_delta_b_x, delta_f_delta_b_y, delta_f_delta_b_z);
					
					a_x -= h * delta_f_delta_a_x;
					a_y -= h * delta_f_delta_a_y;
					a_z -= h * delta_f_delta_a_z;
					
					b_x -= 0.0001 * h * delta_f_delta_b_x;
					b_y -= 0.0001 * h * delta_f_delta_b_y;
					b_z -= 0.0001 * h * delta_f_delta_b_z;
					
					//printf("a_x,a_y,a_z = %f, %f, %f\n", a_x, a_y, a_z);
					//printf("b_x,b_y,b_z = %f, %f, %f\n", b_x, b_y, b_z);
					delta_norm = 50*50 - ((a_x + b_x * x)*(a_x + b_x * x) + (a_y + b_y * y)*(a_y + b_y * y) + (a_z + b_z * z)*(a_z + b_z * z));
					//printf("new local norm = %f\n", delta_norm);
					//printf("___________________\n");
				}
			}
			
			magnet_calibration_data_counter++;
		}
		*/
		
		//updateRotationMatrix(&kite_orientation_data, kite_mpu_raw_data);
		//updateRotationMatrix(&line_orientation_data, line_mpu_raw_data);
		
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
