#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


#include "../../common/i2c_devices/cat24c256.h"
#include "../../common/i2c_devices/bmp280.h"
#include "../../common/i2c_devices/mpu6050.h"

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

#define MAX_SERVO_DEFLECTION 50
#define MAX_BRAKE_DEFLECTION 50
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
	updateBMP280Config(config_values[6]);
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
	
	Orientation_Data orientation_data;
	initRotationMatrix(&orientation_data);
	
	init_cat24(bus1);
	
	//testConfigWriting();//TODO: remove. DEBUGGING ONLY
	
	Mpu_raw_data mpu_calibration = {
		{readEEPROM(0), readEEPROM(1), readEEPROM(2)},
		{readEEPROM(3), readEEPROM(4), readEEPROM(5)}
	};
	
	int output_pins[] = {27,26,12,13,5,15};
	initMotors(output_pins, 6);
	
	setAngle(0, 0);
	setAngle(1, 0);
	setAngle(3, 0);
	setAngle(5, 0);
	setSpeed(2, 0);
	setSpeed(4, 0);
	
    initMPU6050(bus0, mpu_calibration);
	updateRotationMatrix(&orientation_data); // to find out if nose (or wing tip) up or down on initialization
	
	// ************************ KITE WING TIP POINTING UP -> ESC CALIBRATION MODE ************************
	/*
	if(getAccelY() < -7 || getAccelY() > 7){ // m/s**2
		printf("entering ESC calibration mode:\n");
		for(int i = 0; i < 500; i++){
			setSpeed(2, 90);
			setSpeed(4, 90);
			//vTaskDelay(1000);
			vTaskDelay(1);
		}
		printf(" ESCs calibrated\n");
	}*/
	
	// ************************ KITE NOSE POINTING DOWN -> CONFIG MODE ************************
	
	if(getAccelX() < 0){
		printf("entering config mode\n");
		readConfigValuesFromEEPROM(config_values);
		network_setup_configuring(&getConfigValues ,&setConfigValues, &actuatorControl, &orientation_data);
		
		// THIS TAKES TIME...
		float bmp_calib = readEEPROM(6);//-0.000001; // TODO: recalibrate and remove the -0.000001 hack
    	init_bmp280(bus1, bmp_calib);
		
		while(1){
			vTaskDelay(1);
			update_bmp280_if_necessary();
			updateRotationMatrix(&orientation_data);
			if(data_needs_being_written_to_EEPROM == 1){
				writeConfigValuesToEEPROM(config_values);
				data_needs_being_written_to_EEPROM = 0;
			}
		}
	}
	
	// ************************ KITE NOSE POINTING UP -> FLIGHT MODE ************************
	
	printf("Entering flight mode. Excitement guaranteed :D\n");
	network_setup_kite_flying(&setConfigValues);
	
	//int input_pins[] = {4, 33, 2, 17, 16};
	//initPWMInput(input_pins, 5);
	
	
    
    readConfigValuesFromEEPROM(config_values);
    
    // **** WAITING for GROUNDSTATION to ECHO the config array ****
    
    while(!groundstation_has_config_values_initialized_from_kite_EEPROM){
    	sendDataArrayLarge(CONFIG_MODE, config_values, NUM_CONFIG_FLOAT_VARS); // *** SENDING of CONFIG ARRAY via ESP-NOW
    	printf("Sending config array\n");
    	vTaskDelay(100);
    }
    
    // THIS TAKES TIME...
	float bmp_calib = readEEPROM(6);//-0.000001; // TODO: recalibrate and remove the -0.000001 hack
    init_bmp280(bus1, bmp_calib);
	
	initAutopilot(&autopilot, config_values);
	
	//autopilot.mode = FINAL_LANDING_MODE;//EIGHT_MODE;//FINAL_LANDING_MODE; // ONLY FOR DEBUGGING; TODO: REMOVE
	
	int propellerBootState = 0;
	float propellerFactor = 0;
	
	while(1) {
		vTaskDelay(1);
		//printf("mode = %d\n", autopilot.mode);
		if(data_needs_being_written_to_EEPROM == 1){
			writeConfigValuesToEEPROM(config_values);
			data_needs_being_written_to_EEPROM = 0;
		}
		
		update_bmp280_if_necessary();
		
		updateRotationMatrix(&orientation_data);
		
		//updatePWMInput();
		//propellerFactor = getPWMInput0to1normalized(2);
		if(propellerBootState == 0 && getAccelX() < 0){ // kite nose pointing down
			propellerBootState = 1;
			propellerFactor = 0.05;
		}
		if(propellerBootState == 1 && getAccelX() > 0){ // kite nose pointing up
			propellerBootState = 2;
		}
		if(propellerBootState == 2 && propellerFactor < 1){
			propellerFactor = clamp(propellerFactor+0.004, 0, 1);
		}
		
		float line_length = clamp(line_length_in_meters, 0, 1000000); // global var defined in RC.c, should default to 1 when no signal received, TODO: revert line length in VESC LISP code
		autopilot.fm = flight_mode;// global var flight_mode defined in RC.c, 
		//printf("autopilot.mode = %d", autopilot.mode);
		SensorData sensorData;
		initSensorData(&sensorData, orientation_data.rotation_matrix_transpose, orientation_data.gyro_in_kite_coords, getHeight()-groundstation_height, getHeightDerivative());
		
		//TODO: decide size of timestep_in_s in main.c and pass to stepAutopilot(), or use same method as used in updateRotationMatrix
		ControlData control_data;
		
		//DEBUGGING
		stepAutopilot(&autopilot, &control_data, sensorData, line_length, 3/*line tension*/);
		
		// DON'T LET SERVOS BREAK THE KITE
		control_data.brake = clamp(control_data.brake, -MAX_BRAKE_DEFLECTION, MAX_BRAKE_DEFLECTION);
		control_data.left_elevon = clamp(control_data.left_elevon, -MAX_SERVO_DEFLECTION, MAX_SERVO_DEFLECTION);
		control_data.right_elevon = clamp(control_data.right_elevon, -MAX_SERVO_DEFLECTION, MAX_SERVO_DEFLECTION);
		
		//TODO: setAngle in radians ( * PI/180) and setSpeed from [0, 1] or so
		actuatorControl(control_data.left_elevon, control_data.right_elevon, control_data.brake, control_data.rudder, propellerFactor*control_data.left_prop, propellerFactor*control_data.right_prop, MAX_PROPELLER_SPEED);
		
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
