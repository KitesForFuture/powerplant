
#include <math.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "nvs_flash.h"
#include "esp_wifi.h"

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
#include "driver/gpio.h"
#include "driver/uart.h"
#include "uart.c"

#define ESP32_UART 0

float config_values[NUM_CONFIG_FLOAT_VARS];

int data_needs_being_send_via_UART = 0;

float debuggingData[6];
int debuggingDataInitialized = false;

void getDebuggingData(float* values){
	for(int i = 0; i < 6; i++){
		values[i] = debuggingDataInitialized? debuggingData[i] : 1000000;
	}
}

void getConfigValues(float* values){
	for (int i = 0; i < NUM_CONFIG_FLOAT_VARS; i++){
		values[i] = config_values[i];
	}
}

void setConfigValues(float* values){
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
		config_values[i] = values[i];
	}
	data_needs_being_send_via_UART = 1;
}

void app_main(void)
{
	initUART(ESP32_UART, GPIO_NUM_19, GPIO_NUM_18, false);
	//initializeConfigValues(config_values);
	
	int length = 0;
	float receive_array[200];
	int network_initialized = false;
	printf("waiting for config data from kite...\n");
	while(1){
		//printf("looping\n");
		//sendUART(1,2, ESP32_UART);//DEBUGGING
		//sendUARTArray100(config_values, NUM_CONFIG_FLOAT_VARS, ESP32_UART);//DEBUGGING
		if(data_needs_being_send_via_UART == 1){
			sendUARTArray100(config_values, NUM_CONFIG_FLOAT_VARS, ESP32_UART);
			data_needs_being_send_via_UART = 0;
		}
		length = processUART(ESP32_UART, receive_array);
		
		// *** ECHO config array and INITIALIZE values[] ***
		if (length == NUM_CONFIG_FLOAT_VARS) {
			printf("Received config from kite: ");
			for(int i = 6; i < NUM_CONFIG_FLOAT_VARS; i++){
				config_values[i] = receive_array[i];
				printf("%f, ", config_values[i]);
			}
			printf("\n");
			//now that we have the config values from the kite EEPROM, we can broadcast the config website as an Access Point
			if(!network_initialized){
				printf("setting up access point\n");
				network_setup_configuring(&setConfigValues, &getConfigValues, &getDebuggingData);
				network_initialized = true;
			}
			
			//ECHO config_values to UART
			printf("Echoing config values back to groundstation\n");
			sendUARTArray100(config_values, NUM_CONFIG_FLOAT_VARS, ESP32_UART);
		}else if (length == 2){
			//ignore, this is status message for internet
		}else if (length == 6){
			for(int i = 0; i < 6; i++) debuggingData[i] = receive_array[i];
			debuggingDataInitialized = true;
			printf("received debugging data\n");
		}
		
		vTaskDelay(1);
	}
}
