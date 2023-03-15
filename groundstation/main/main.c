/* WIFI-UART translation
*/

#include <math.h>
#include <string.h>
#include "timer.c"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"

#include "driver/uart.h"

#include "driver/gpio.h"

#include "driver/mcpwm.h"

#include "mymath.c"


#include "../../common/i2c_devices/cat24c256.h"
#include "../../common/i2c_devices/bmp280.h"

#include "../../common/uart.c"

#include "motors.c"

#include "my_gpio.c"

#include "../../common/RC.c"



#define SERVO_MIN_ANGLE -45
#define SERVO_MAX_ANGLE 54

#define NOT_INITIALIZED -1000000

#define VESC_UART 0
#define ESP32_UART 1

#define LAND_COMMAND 0
#define LAUNCH_COMMAND 1

struct i2c_bus bus0 = {18, 19};//SDA, SCL

float bmp_calib = 0;

int internet_connected = false;

float currentServoAngle = SERVO_MIN_ANGLE;
float direction = 1;

void set_bmp_calibration(float value){
	//printf("setting bmp_calibration(gs) to %f * 10**-5\n", value*100000);
	float readValue = readEEPROM(0);
	if(value != readValue){
		write2EEPROM(value, 0);
	}
	bmp_calib = value;
	updateBMP280Config(value);
}

void controlServoAngle(float reel_in_speed){
	currentServoAngle += direction*0.0045*reel_in_speed;
	if(currentServoAngle > SERVO_MAX_ANGLE) {direction = -1; currentServoAngle = SERVO_MAX_ANGLE;}
	if(currentServoAngle < SERVO_MIN_ANGLE) {direction = 1; currentServoAngle = SERVO_MIN_ANGLE;}
	printf("setting servo angle = %f\n", currentServoAngle);
	setAngle(2, currentServoAngle);
}

void storeServoArmForEnergyGeneration(){
	setAngle(2, -70);
	currentServoAngle = SERVO_MIN_ANGLE;
	direction = 1;
}


void processReceivedConfigValuesViaWiFi(float* config_values){
	//TODO: send UART with config values
	printf("Received config over ESP-NOW.\n");
	if(internet_connected){
		printf("ECHOING back to ESP-NOW, because INTERNET_CONTROL ESP is connected");
		sendDataArrayLarge(CONFIG_MODE, config_values, NUM_CONFIG_FLOAT_VARS); // ECHO the config array
	}else{
		printf("FORWARDING as UART on PIN 16\n");
		//forwarding config, while appending groundstation-bmp280-config
		float config_values_kite_and_gs[NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS];
		for(int i = 0; i < NUM_CONFIG_FLOAT_VARS; i++) config_values_kite_and_gs[i] = config_values[i];
		config_values_kite_and_gs[NUM_CONFIG_FLOAT_VARS + 0] = bmp_calib;
		sendUARTArray100(config_values_kite_and_gs, NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS, ESP32_UART); // FORWARD config (appending groundstation config) to access point ESP32
	}
}

void processReceivedDebuggingDataViaWiFi(float* debugging_data){
	//printf("forwarding Debugging data to in_flight_config ESP32");
	sendUARTArray100(debugging_data, 6, ESP32_UART);
}

void init(){
	
	init_cat24(bus0);
	bmp_calib = readEEPROM(0);
    init_bmp280(bus0, bmp_calib);
	
	initMotors(); // servo (pwm) outputs
	storeServoArmForEnergyGeneration();
	//networking
	//setRole(GROUND_STATION);
	
	// UART TO VESC/STM32
	initUART(VESC_UART, GPIO_NUM_12, GPIO_NUM_13, true);
	// UART TO wifi station/ap ESP32s
	initUART(ESP32_UART, GPIO_NUM_16, GPIO_NUM_17, false);
	
	network_setup_groundstation(&processReceivedConfigValuesViaWiFi, &processReceivedDebuggingDataViaWiFi);
	
	initGPIO();
	
	init_uptime();
	
}

float line_length = 0;
float line_speed = 0;
float line_length_offset = NOT_INITIALIZED;
float last_line_length = 0;

void app_main(void){
	init();
	//storeServoArmForEnergyGeneration();
	//controlServoAngle(30.0 * reel_in_high_duty_voltage);
	//float landing_request = 0.0;
	float receive_array[100];
	int receive_array_length = 0;
	float line_length_raw, flight_mode;
	printf("waiting for UART...\n");
	while(1){
		update_bmp280_if_necessary();
		//printf("getHeight() = %f\n", getHeight());
		//sendUART(1, 2, VESC_UART);//DEBUGGING
		//sendUART(1, 2, ESP32_UART);//DEBUGGING
		// **************** REACT to UART message from VESC ****************
		
		// pass line length and line tension from UART to WIFI
		receive_array_length = processUART(VESC_UART, receive_array);
		if(receive_array_length == 2){
			printf("received UART from VESC\n");
			line_length_raw = receive_array[0];
			flight_mode = receive_array[1];
		
			line_length = line_length_raw;// - line_length_offset;
			//printf("received %f, %f\n", line_length_raw, flight_mode);
			printf("sending flight_mode %f and line_length %f to kite\n", flight_mode, line_length);
			sendData(LINE_LENGTH_MODE, line_length, flight_mode, getHeight()); // send line_length, flight_mode to kite
			printf("sending flight_mode %f and line_length %f to communication ESP32\n", flight_mode, line_length);
			if(internet_connected){
				sendUART(flight_mode, line_length, ESP32_UART); // send flight_mode to attached ESP32, which forwards it to the internet
			}
			line_speed = (1-0.125) * line_speed + 0.125 * 50/*frequency*/ * (line_length - last_line_length);
			last_line_length = line_length;
			//printf("line_speed = %f, line_length = %f\n", line_speed, line_length);
			if(-line_speed > 0.5){
				controlServoAngle(30.0 * fabs(line_speed));
			}else{
				storeServoArmForEnergyGeneration();
			}
		}
		
		// **************** REACT to UART message from ESP32 ****************
		
		// receiving from attached ESP32 via UART
		receive_array_length = processUART(ESP32_UART, receive_array);
		if(receive_array_length == 2){ // received from INTERNET
			printf("Sending landing/launching request to VESC: %f\n", receive_array[0]);
			sendUART(receive_array[0], 0, VESC_UART); // landing (TODO: launch) COMMAND
			internet_connected = true;
		}else if(receive_array_length == NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS){ // received from in_flight_config CONFIG TOOL
			printf("sending config to kite via ESP-NOW\n");
			sendDataArrayLarge(CONFIG_MODE, receive_array, NUM_CONFIG_FLOAT_VARS); // *** FORWARD of CONFIG ARRAY from UART to ESP-NOW, stripping groundstation config
			set_bmp_calibration(receive_array[NUM_CONFIG_FLOAT_VARS+0]);
		}
		
		// **************** MANUAL SWITCH ****************
		
		if(!internet_connected){
			if(get_level_GPIO_0() != 0){
				printf("SWITCH request final landing\n");
				sendUART(1, 0, VESC_UART); // request final-landing from VESC
			}else{
				sendUART(0, 0, VESC_UART);
			}
		}
		
	    //sendUART(1, 0, VESC_UART); // request final-landing from VESC
	    vTaskDelay(5.0);
    }
    
}
