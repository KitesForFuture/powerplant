#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


#include "../../common/i2c_devices/cat24c256.h"
//#include "../../common/i2c_devices/mpu6050.h"
#include "../../common/i2c_devices/mpu9250.h"
#include "../../common/i2c_devices/dps310.h"

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
	init_dps310(bus1);
	
	
	Mpu_raw_data_9250 line_mpu_calibration = {
		{0, 0, 0},
		{1.66, 0.81, 0.05},
		{62.9, 6.9, 18.9}
	};
	
	int output_pins[] = {27,26,12,13,5,15};
	initMotors(output_pins, 6);
	
	setAngle(0, 0);
	setAngle(1, 0);
	setAngle(3, 0);
	setAngle(5, 0);
	setSpeed(2, 0);
	setSpeed(4, 0);
	
	
	MPU9250 line_mpu;
	
	line_mpu.bus = bus0;
	
	line_mpu.address = 105;
	line_mpu.magnetometer_address = 12;
	
	line_mpu.calibration_data = line_mpu_calibration;
	
    initMPU9250(&line_mpu);
    
	Mpu_raw_data_9250 line_mpu_raw_data = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};
	
	readConfigValuesFromEEPROM(config_values);
	network_setup_configuring(&getConfigValues ,&setConfigValues, &actuatorControl, &kite_orientation_data, &line_orientation_data);
	
	float mag_x[128];
	float mag_y[128];
	float mag_z[128];
	int magnet_calibration_data_counter = 0;
	
	while(1) {
		vTaskDelay(1);
		update_dps310_if_necessary();
		
		readMPUData9250(&line_mpu, &line_mpu_raw_data);
		
		updateRotationMatrix(&line_orientation_data, line_mpu_raw_data);
		float* mat = line_orientation_data.rotation_matrix;
		
		updateRotationMatrix(&line_orientation_data, line_mpu_raw_data);
		
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
