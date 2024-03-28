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

#define MAX_SERVO_DEFLECTION 60//50
#define MIN_BRAKE_DEFLECTION -59//-64
#define MAX_BRAKE_DEFLECTION 59
#define MAX_PROPELLER_SPEED 90 // AT MOST 90

struct i2c_bus bus0 = {18, 19};
struct i2c_bus bus1 = {25, 14};

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
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
		config_values[i] = values[i];
	}
	
	data_needs_being_written_to_EEPROM = 1;
	groundstation_has_config_values_initialized_from_kite_EEPROM = true;
	loadConfigVariables(&autopilot, config_values);
}

void actuatorControl(float left_aileron, float right_aileron, float left_elevon, float right_elevon, float brake, float rudder, float left_propeller, float right_propeller, float propeller_safety_max){
	printf("right_aileron = %f\n", right_aileron);
	if(config_values[52]){ // SWAPPED
		setAngle(6, config_values[48] + config_values[50]*left_aileron); // left aileron
		setAngle(7, config_values[49] + config_values[51]*right_aileron); // right aileron
	}else{
		setAngle(7, config_values[48] + config_values[50]*left_aileron); // left aileron
		setAngle(6, config_values[49] + config_values[51]*right_aileron); // right aileron
	}
	
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
	
	init_cat24(bus1);
	
	/*
	Mpu_raw_data kite_mpu_calibration = {
		{readEEPROM(0), readEEPROM(1), readEEPROM(2)},
		{readEEPROM(3), readEEPROM(4), readEEPROM(5)}
	};
	
	Mpu_raw_data line_mpu_calibration = {
		{readEEPROM(1024+0), readEEPROM(1024+1), readEEPROM(1024+2)},
		{readEEPROM(1024+3), readEEPROM(1024+4), readEEPROM(1024+5)}
	};
	*/
	
	raw_data_ICM20948 kite_and_line_mpu_calibration = {
		{0.132-0.135, 0.12, 0.135-0.31}, // accel
		{readEEPROM(3), readEEPROM(4), readEEPROM(5)}, // gyro
		//{52.76, 50.66, -50.33},//{1.74, 0.93, 0.08},
		{65.6, 6.8, 22.5} // magnet
	};
	
	
	int output_pins[] = {27,23, 2/*prop*/, 12, 15/*prop*/,13,17,16};
	initMotors(output_pins, 8);
	
	setAngle(0, 0);
	setAngle(1, 0);
	setAngle(3, 0);
	setAngle(5, 0);
	setSpeed(2, 0);
	setSpeed(4, 0);
	setAngle(6, 0);
	setAngle(7, 0);
	//setSpeed(2, 90);
	//setSpeed(4, 90);
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
	
	
	// ************************ KITE WING TIP POINTING UP -> ESC CALIBRATION MODE (TODO: get it right) ************************
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
	
	float avg_x = 0;
	float avg_y = 0;
	float avg_z = 0;
	int numGyroCalibrationSteps = 100;
	int gyroCalibrationStepsOutstanding = 0;
	
	if(true || getAccelX(kite_and_line_mpu_raw_data) < 0){
		printf("entering config mode\n");
		readConfigValuesFromEEPROM(config_values);
		network_setup_configuring(&getConfigValues ,&setConfigValues, &actuatorControl, &kite_orientation_data);
		
		init_dps310(bus1);
		Time t = start_timer();
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		while(1){
			//vTaskDelay(0);
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
			/*
			avg_x = avg_x*0.95+kite_and_line_mpu_raw_data.gyro[0]*0.05;
			avg_y = avg_y*0.95+kite_and_line_mpu_raw_data.gyro[1]*0.05;
			avg_z = avg_z*0.95+kite_and_line_mpu_raw_data.gyro[2]*0.05;
			printf("gyro = %f, %f, %f\n", avg_x, avg_y, avg_z);
			*/
			//printf("z-comp of mag = %f\n", kite_and_line_mpu_raw_data.magnet[2]);
			
			//printf("z-axis up? = %f\n", -kite_orientation_data.rotation_matrix_transpose[3]);
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
    readConfigValuesFromEEPROM(config_values);
    
    // **** WAITING for GROUNDSTATION to ECHO the config array ****
    
    while(!groundstation_has_config_values_initialized_from_kite_EEPROM){
    	sendDataArrayLarge(CONFIG_MODE, config_values, NUM_CONFIG_FLOAT_VARS); // *** SENDING of CONFIG ARRAY via ESP-NOW
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
	
	while(1) {
		vTaskDelay(1);
		//printf("mode = %d\n", autopilot.mode);
		if(data_needs_being_written_to_EEPROM == 1){
			printf("writing config to eeprom");
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
			propellerFactor = clamp(propellerFactor+0.001, 0, 1);
		}
		
		float line_length = clamp(line_length_in_meters, 0, 1000000); // global var defined in RC.c, should default to 1 when no signal received, TODO: revert line length in VESC LISP code
		autopilot.fm = flight_mode;// global var flight_mode defined in RC.c, 
		//printf("autopilot.mode = %d", autopilot.mode);
		SensorData sensorData;
		initSensorData(&sensorData, kite_orientation_data.rotation_matrix_transpose, kite_orientation_data.line_vector_normed, kite_orientation_data.gyro_in_kite_coords, getHeight()-groundstation_height, getHeightDerivative());
		
		//TODO: decide size of timestep_in_s in main.c and pass to stepAutopilot(), or use same method as used in updateRotationMatrix
		ControlData control_data;
		
		//autopilot.mode = EIGHT_MODE;
		//DEBUGGING
		stepAutopilot(&autopilot, &control_data, sensorData, line_length, 3/*line tension*/);
		
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
