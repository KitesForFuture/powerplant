#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


#include "../../common/i2c_devices/cat24c256.h"
#include "../../common/i2c_devices/dps310.h"
#include "../../common/i2c_devices/icm20948.h"

#include "control/rotation_matrix.h"
#include "../../common/pwm/motors.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "driver/uart.h"
#include "../../common/RC.c"
//#include "../../common/RC_proxy.c"

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

#include "../../common/helpers/adc.h"

#define MAX_SERVO_DEFLECTION 60.0//50
#define MIN_BRAKE_DEFLECTION -59.0//-64
#define MAX_BRAKE_DEFLECTION 59.0
#define MAX_PROPELLER_SPEED 90.0 // AT MOST 90

#define HEIGHT_CALIBRATION_OFFSET 1.0

struct i2c_bus bus0 = {18, 19};
struct i2c_bus bus1 = {25, 14};

static Autopilot autopilot;

float config_values[NUM_CONFIG_FLOAT_VARS];
int config_values_changed_mask[NUM_CONFIG_FLOAT_VARS];

int data_needs_being_written_to_EEPROM = 0;

void writeConfigValuesToEEPROM(float* values){
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
		if(config_values_changed_mask[i]){
			write2EEPROM(values[i], i);
			config_values_changed_mask[i] = false;
		}
	}
	//printf("values[6] to write = %f\n", values[6]);
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
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
		if(config_values[i] != values[i]){
			config_values[i] = values[i];
			config_values_changed_mask[i] = true;
		}
	}
	
	data_needs_being_written_to_EEPROM = 1;
	groundstation_has_config_values_initialized_from_kite_EEPROM = true;
	loadConfigVariables(&autopilot, config_values);
}

void actuatorControl(float left_aileron, float right_aileron, float left_elevon, float right_elevon, float brake, float rudder, float left_propeller, float right_propeller, float propeller_safety_max){
	
	if(config_values[52]){ // SWAPPED
		setAngle(6, config_values[48] + config_values[50]*left_aileron); // left aileron
		setAngle(7, config_values[49] + config_values[51]*right_aileron); // right aileron
	}else{
		setAngle(7, config_values[48] + config_values[50]*left_aileron); // left aileron
		setAngle(6, config_values[49] + config_values[51]*right_aileron); // right aileron
	}
	//setAngle(7, -5 + left_aileron); // left aileron
	//setAngle(6, 5 + -right_aileron); // right aileron
	//printf("%f, %f,%f, %f,%f\n", config_values[50], config_values[51], config_values[52], config_values[48], config_values[49]);
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
	
	initADC();
	
	Orientation_Data kite_orientation_data;
	initRotationMatrix(&kite_orientation_data);
	
	init_cat24(bus1);
	
	raw_data_ICM20948 kite_and_line_mpu_calibration = {
		{0.0, 0.0, 0.0}, // accel
		{readEEPROM(3), readEEPROM(4), readEEPROM(5)}, // gyro
		{0.0, 0.0, 0.0} // magnet
	};
	
	int output_pins[] = {23,17, 2/*prop*/, 27, 15/*prop*/,13,12,33};
	initMotors(output_pins, 8);
	
	ICM20948 kite_and_line_mpu;
	
	kite_and_line_mpu.bus = bus0;
	kite_and_line_mpu.address = 0x68;
	kite_and_line_mpu.magnetometer_address = 12;
	kite_and_line_mpu.calibration_data = kite_and_line_mpu_calibration;
    initICM20948(&kite_and_line_mpu);
    printf("initialized icm-20948");
    
    raw_data_ICM20948 kite_and_line_mpu_raw_data = {
		{0, 0, 0},
		{0, 0, 0},
		{0, 0, 0}
	};
	readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
	updateRotationMatrix(&kite_orientation_data, kite_and_line_mpu_raw_data);
	
	
	readConfigValuesFromEEPROM(config_values);
	for(int i = 0; i < NUM_CONFIG_FLOAT_VARS; i++){
		config_values_changed_mask[i] = false;
	}
	
	setAngle(0, 0);
	setAngle(1, config_values[10]*MIN_BRAKE_DEFLECTION);
	setAngle(3, 0);
	setAngle(5, 0);
	setSpeed(2, 0);
	setSpeed(4, 0);
	setAngle(6, 0);
	setAngle(7, 0);
	
	// ************************ KITE NOSE POINTING DOWN -> CONFIG MODE ************************
	
	float avg_x = 0;
	float avg_y = 0;
	float avg_z = 0;
	int numGyroCalibrationSteps = 100;
	int gyroCalibrationStepsOutstanding = 0;
	
	if(getAccelX(kite_and_line_mpu_raw_data) < 0){
		printf("entering config mode\n");
		network_setup_configuring(&getConfigValues ,&setConfigValues, &actuatorControl, &kite_orientation_data);
		
		init_dps310(bus1);
		Time t = start_timer();
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		float voltage = 0;
		while(1){
			float U = 15.08 + 0.007062 * (getVoltageInMilliVolt() - 1909);
			voltage = 0.05 * U + 0.95 * voltage;
			printf("Raw Voltage sensing value: %f\n", voltage);
			//vTaskDelay(0);
			//printf("roll angle = %f\n", getLineRollAngle(kite_orientation_data.line_vector_normed));
			//printf("yaw angle = %f\n", getLineYawAngle(kite_orientation_data.line_vector_normed));
			
			vTaskDelayUntil(&xLastWakeTime, 2);
			float timestep = get_time_step(&t);
			//printf("timestep = %f\n", timestep);
			
			update_dps310_if_necessary();
			
			if(gyroCalibrationCommand){
				gyroCalibrationCommand = false;
				gyroCalibrationStepsOutstanding = numGyroCalibrationSteps;
				avg_x = 0;
				avg_y = 0;
				avg_z = 0;
			}
			
			if(gyroCalibrationStepsOutstanding > 0){
				readRawDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
				avg_x += kite_and_line_mpu_raw_data.gyro[0];
				avg_y += kite_and_line_mpu_raw_data.gyro[1];
				avg_z += kite_and_line_mpu_raw_data.gyro[2];
				printf("gyro = %f, %f, %f\n", kite_and_line_mpu_raw_data.gyro[0], kite_and_line_mpu_raw_data.gyro[1], kite_and_line_mpu_raw_data.gyro[2]);
				gyroCalibrationStepsOutstanding -= 1;
			}else if(!gyroCalibrated){
				avg_x *= 1.0/numGyroCalibrationSteps;
				avg_y *= 1.0/numGyroCalibrationSteps;
				avg_z *= 1.0/numGyroCalibrationSteps;
				
				// save to EEPROM
				vTaskDelay(10);
				write2EEPROM(avg_x, 3);
				vTaskDelay(10);
				write2EEPROM(avg_y, 4);
				vTaskDelay(10);
				write2EEPROM(avg_z, 5);
				vTaskDelay(10);
				
				printf("gyro calibration = %f, %f, %f\n", avg_x, avg_y, avg_z);
				printf("eeprom state = %f, %f, %f\n", readEEPROM(3), readEEPROM(4), readEEPROM(5));
				
				// use now
				kite_and_line_mpu.calibration_data.gyro[0] = avg_x;
				kite_and_line_mpu.calibration_data.gyro[1] = avg_y;
				kite_and_line_mpu.calibration_data.gyro[2] = avg_z;
				
				gyroCalibrated = true;
			}else{
				readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
				updateRotationMatrix(&kite_orientation_data, kite_and_line_mpu_raw_data);
			}
			if(data_needs_being_written_to_EEPROM == 1){
				writeConfigValuesToEEPROM(config_values);
				data_needs_being_written_to_EEPROM = 0;
			}
		}
	}
	
	// ************************ KITE NOSE POINTING UP -> FLIGHT MODE ************************
	
	printf("Entering flight mode. Excitement guaranteed :D\n");
	network_setup_kite_flying(&setConfigValues);
	
    printf("reading config values from eeprom\n");
    readConfigValuesFromEEPROM(config_values); // TODO: obsolete???
    
    // **** WAITING for GROUNDSTATION to ECHO the config array ****
    
    while(!groundstation_has_config_values_initialized_from_kite_EEPROM){
    	sendDataArrayLarge(CONFIG_MODE, config_values, NUM_CONFIG_FLOAT_VARS); // *** SENDING of CONFIG ARRAY via ESP-NOW
    	processRC();
    	printf("Sending config array\n");
    	vTaskDelay(100);
    }
    
    printf("initializing dps310\n");
    
    vTaskDelay(10);
    init_dps310(bus1);
    vTaskDelay(10);
    
	initAutopilot(&autopilot, config_values);
	
	//autopilot.mode = TEST_MODE;//autopilot.mode = EIGHT_MODE;//FINAL_LANDING_MODE;//EIGHT_MODE;//FINAL_LANDING_MODE; // ONLY FOR DEBUGGING; TODO: REMOVE
	
	int propellerBootState = -200;
	float propellerFactor = 0;
	
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	
	Time t = start_timer();
	float timestep = 0;
	while(1) {
		
		vTaskDelayUntil(&xLastWakeTime, 2);
		
		float new_timestep = 1024*get_time_step(&t);
		timestep = 0.9*timestep + 0.1 * new_timestep * new_timestep;
		//sendDebuggingData(timestep, getHeight(), groundstation_height, getHeight()-groundstation_height+HEIGHT_CALIBRATION_OFFSET, getHeightDerivative(), 0);
		processRC();
		//printf("mode = %d\n", autopilot.mode);
		if(data_needs_being_written_to_EEPROM == 1){
			//printf("writing config to eeprom");
			writeConfigValuesToEEPROM(config_values);
			data_needs_being_written_to_EEPROM = 0;
		}
		
		update_dps310_if_necessary();
		
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		updateRotationMatrix(&kite_orientation_data, kite_and_line_mpu_raw_data);
		
		if(propellerBootState < 0 && getAccelX(kite_and_line_mpu_raw_data) < 0){ // kite nose pointing down
			propellerBootState++;
		}
		if(propellerBootState == 0){ // kite nose pointing down
			propellerBootState = 1;
			propellerFactor = 0.2;
		}
		if(propellerBootState == 1 && getAccelX(kite_and_line_mpu_raw_data) > 0){ // kite nose pointing up
			propellerBootState = 2;
		}
		if(propellerBootState == 2 && propellerFactor < 1){
			propellerFactor = clamp(propellerFactor+0.004, 0, 1); //4 seconds for propellerFactor from 0.2 to 1 at 50Hz, TOOD: danger of overflow. But is float, might take very long time.
		}
		
		float line_length = line_length_in_meters;//clamp(line_length_in_meters, 0, 1000000); // global var defined in RC.c, should default to 1 when no signal received, TODO: revert line length in VESC LISP code
		autopilot.fm = flight_mode;// global var flight_mode defined in RC.c, 
		//printf("autopilot.mode = %d", autopilot.mode);
		SensorData sensorData;
		initSensorData(&sensorData, kite_orientation_data.rotation_matrix_transpose, kite_orientation_data.line_vector_normed, kite_orientation_data.gyro_in_kite_coords, getHeight()-groundstation_height+HEIGHT_CALIBRATION_OFFSET, getHeightDerivative());
		
		//TODO: decide size of timestep_in_s in main.c and pass to stepAutopilot(), or use same method as used in updateRotationMatrix
		ControlData control_data;
		
		//autopilot.mode = EIGHT_MODE;
		//DEBUGGING, TODO: remove
		//line_length = 3;
		stepAutopilot(&autopilot, &control_data, sensorData, line_length, line_speed, 3/*line tension*/);
		
		// DON'T LET SERVOS BREAK THE KITE
		control_data.brake = clamp(control_data.brake, MIN_BRAKE_DEFLECTION, MAX_BRAKE_DEFLECTION);
		control_data.left_elevon = clamp(control_data.left_elevon, -MAX_SERVO_DEFLECTION, MAX_SERVO_DEFLECTION);
		control_data.right_elevon = clamp(control_data.right_elevon, -MAX_SERVO_DEFLECTION, MAX_SERVO_DEFLECTION);
		
		//TODO: setAngle in radians ( * PI/180) and setSpeed from [0, 1] or so
		actuatorControl(control_data.left_aileron, control_data.right_aileron, control_data.left_elevon, control_data.right_elevon, control_data.brake, control_data.rudder, propellerFactor*control_data.left_prop, propellerFactor*control_data.right_prop, MAX_PROPELLER_SPEED);
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
