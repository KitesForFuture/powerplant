#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"


#include "../../common/i2c_devices/cat24c256.h"
#include "../../common/i2c_devices/dps310.h"
#include "../../common/i2c_devices/icm20948.h"

#include "control/rotation_matrix.h"
#include "../../common/pwm/motors.h"
#include "../../common/pwm/pwm_input.h"

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

#include "my_gpio.c"
#include "driver/ledc.h"

#define MAX_SERVO_DEFLECTION 60.0//50
#define MIN_BRAKE_DEFLECTION -59.0//-64
#define MAX_BRAKE_DEFLECTION 59.0
#define MAX_PROPELLER_SPEED 90.0 // AT MOST 90

#define HEIGHT_CALIBRATION_OFFSET 1.0


#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO_0        (2) // Define the output GPIO
#define LEDC_OUTPUT_IO_1        (33) // Define the output GPIO
#define LEDC_CHANNEL_0          LEDC_CHANNEL_0
#define LEDC_CHANNEL_1          LEDC_CHANNEL_1
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz


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
			printf("writing %f to memory location [%d]\n", values[i], i);
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

void actuatorControl(float steering, float max_angle){
	setAngle(0, config_values[41] + config_values[40]*clamp(steering, -max_angle, max_angle)); // Rudder
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

static void example_ledc_init(void)
{
    // Prepare and then apply the LEDC PWM timer configuration
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // Prepare and then apply the LEDC PWM channel configuration
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_0,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
    
    ledc_channel_config_t ledc_channel2 = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL_1,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO_1,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel2));
}

void main_task(void* arg)
{
	init_uptime();
	
	initADC();
	
	Orientation_Data kite_orientation_data;
	initRotationMatrix(&kite_orientation_data);
	printf("initializing cat24\n");
	init_cat24(bus1);
	printf("finished initializing cat24\n");
	
	raw_data_ICM20948 kite_and_line_mpu_calibration = {
		{0.0, 0.0, 0.0}, // accel
		{readEEPROM(3), readEEPROM(4), readEEPROM(5)}, // gyro
		{0.0, 0.0, 0.0} // magnet
	};
	
	int output_pins[] = {27};
	initMotors(output_pins, 1);
	
	initGPIO();
	int input_pins[] = {26};
	initPWMInput(input_pins, 1);
	
	ICM20948 kite_and_line_mpu;
	
	kite_and_line_mpu.bus = bus0;
	kite_and_line_mpu.address = 0x68;
	kite_and_line_mpu.magnetometer_address = 12;
	kite_and_line_mpu.calibration_data = kite_and_line_mpu_calibration;
	
	printf("initializing ICM20948\n");
    initICM20948(&kite_and_line_mpu);
	printf("finished initializing ICM20948\n");
    
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
	
	// ************************ KITE NOSE POINTING DOWN -> CONFIG MODE ************************
	
	float avg_x = 0;
	float avg_y = 0;
	float avg_z = 0;
	int numGyroCalibrationSteps = 100;
	int gyroCalibrationStepsOutstanding = 0;
	
	if(getAccelX(kite_and_line_mpu_raw_data) < 0){
		printf("entering config mode\n");
		network_setup_configuring(&getConfigValues ,&setConfigValues, &actuatorControl, &kite_orientation_data);
		
		
		dps_address = 0x77;
		printf("initializing dps310\n");
		init_dps310(bus1);
		printf("finished initializing dps310\n");
		Time t = start_timer();
		TickType_t xLastWakeTime;
		xLastWakeTime = xTaskGetTickCount();
		float voltage = 0;
		while(1){
			float U = 15.08 + 0.007062 * (getVoltageInMilliVolt() - 1909);
			voltage = 0.05 * U + 0.95 * voltage;
			//printf("Raw Voltage sensing value: %f\n", voltage);
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
				//printf("accel raw: %f, %f, %f, gyro raw: %f, %f, %f, mag raw: %f, %f, %f\n", kite_and_line_mpu_raw_data.accel[0], kite_and_line_mpu_raw_data.accel[1], kite_and_line_mpu_raw_data.accel[2], kite_and_line_mpu_raw_data.gyro[0], kite_and_line_mpu_raw_data.gyro[1], kite_and_line_mpu_raw_data.gyro[2], kite_and_line_mpu_raw_data.magnet[0], kite_and_line_mpu_raw_data.magnet[1], kite_and_line_mpu_raw_data.magnet[2]);
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
    /*
    while(!groundstation_has_config_values_initialized_from_kite_EEPROM){
    	sendDataArrayLarge(CONFIG_MODE, config_values, NUM_CONFIG_FLOAT_VARS); // *** SENDING of CONFIG ARRAY via ESP-NOW
    	processRC();
    	printf("Sending config array\n");
    	vTaskDelay(100);
    }
    */
    printf("initializing dps310\n");
    
    vTaskDelay(10);
	dps_address = 0x77;
    init_dps310(bus1);
    vTaskDelay(10);
    
	initAutopilot(&autopilot, config_values);
	
	autopilot.mode = EIGHT_MODE;////autopilot.mode = TEST_MODE;//FINAL_LANDING_MODE;//EIGHT_MODE;//FINAL_LANDING_MODE; // ONLY FOR DEBUGGING; TODO: REMOVE
	
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	
	Time t = start_timer();
	float timestep = 0;
	
	example_ledc_init();
	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, 0);
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, 0);
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
	
	int loop_running_state = 0;
	while(1) {
		
		vTaskDelayUntil(&xLastWakeTime, 2);
		
		//set_level_GPIO_16(rec_led_state);
		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_0, rec_led_state*LEDC_DUTY);
		ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_0);
		if(loop_running_state == 0){
			loop_running_state = 1;
		}else{
			loop_running_state = 0;
		}
		//set_level_GPIO_32(loop_running_state);
		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL_1, loop_running_state*LEDC_DUTY);
		ledc_update_duty(LEDC_MODE, LEDC_CHANNEL_1);
		
		//float new_timestep = 1024*get_time_step(&t);
		//timestep = 0.9*timestep + 0.1 * new_timestep * new_timestep;
		//sendDebuggingData(timestep, getHeight(), groundstation_height, getHeight()-groundstation_height+HEIGHT_CALIBRATION_OFFSET, getHeightDerivative(), 0);
		
		processRC();
		//printf("mode = %d\n", autopilot.mode);
		if(data_needs_being_written_to_EEPROM == 1){
			//printf("writing config to eeprom");
			writeConfigValuesToEEPROM(config_values);
			data_needs_being_written_to_EEPROM = 0;
		}
		
		update_dps310_if_necessary();
		updatePWMInput();
		float pwm_input_value = getPWMInputMinus1to1normalized(0);
		/*if(pwm_input_value < 0.5){
			
		}else{
			set_level_GPIO_32(1);
		}*/
		set_level_GPIO_32(1); // setting source of SRV OUT to GPIO27
		readDataICM20948(&kite_and_line_mpu, &kite_and_line_mpu_raw_data);
		updateRotationMatrix(&kite_orientation_data, kite_and_line_mpu_raw_data);
		
		
		float line_length = clamp(line_length_in_meters, 0, 1000000); // global var defined in RC.c, should default to 1 when no signal received, TODO: revert line length in VESC LISP code
		autopilot.fm = flight_mode;// global var flight_mode defined in RC.c, 
		printf("autopilot.mode = %d\n", autopilot.mode);
		SensorData sensorData;
		initSensorData(&sensorData, kite_orientation_data.rotation_matrix_transpose, kite_orientation_data.line_vector_normed, kite_orientation_data.gyro_in_kite_coords, getHeight()-groundstation_height+HEIGHT_CALIBRATION_OFFSET, getHeightDerivative());
		
		//TODO: decide size of timestep_in_s in main.c and pass to stepAutopilot(), or use same method as used in updateRotationMatrix
		ControlData control_data;
		
		//autopilot.mode = EIGHT_MODE;
		//DEBUGGING, TODO: remove
		//line_length = 3;
		stepAutopilot(&autopilot, &control_data, sensorData, line_length, line_speed, 3/*line tension*/);
		printf("rudder = %f\n", control_data.rudder);
		//TODO: setAngle in radians ( * PI/180) and setSpeed from [0, 1] or so
		actuatorControl(control_data.rudder, 90/*MAX_SERVO_ANGLE*/);
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
