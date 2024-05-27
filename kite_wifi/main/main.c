#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

// for Access Point
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_log.h"
#include "lwip/err.h"
#include "lwip/sys.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "../../common/RC.c"
#include "../../common/helpers/timer.h"


// for http Web Server
#include <sys/param.h>
#include "esp_netif.h"
#include <esp_http_server.h>

#include "driver/gpio.h"
#include "driver/uart.h"
#include "../../common/uart.c"

#define ESP32_UART_WIFI_PROXY 1

void setConfigValues_proxy(float* values){
	printf("received config values via WiFi, forwarding to UART\n");
	sendUARTArray100(values, NUM_CONFIG_FLOAT_VARS, ESP32_UART_WIFI_PROXY);
}

void main_task(void* arg)
{	
	
	init_uptime();
	
	// UART TO autopilot ESP32
	initUART(ESP32_UART_WIFI_PROXY, GPIO_NUM_18, GPIO_NUM_19, false);
	
	network_setup_kite_flying(&setConfigValues_proxy);
	
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	
	int length = 0;
	float receive_array[200];

	while(1) {
		
		vTaskDelayUntil(&xLastWakeTime, 2);
		
		length = processUART(ESP32_UART_WIFI_PROXY, receive_array);
		printf("received somtehing via UART of length %d\n", length);
		
		// forwarding config data UART -> WIFI
		if (length == NUM_CONFIG_FLOAT_VARS) {
			sendDataArrayLarge(CONFIG_MODE, receive_array, NUM_CONFIG_FLOAT_VARS); // *** SENDING of CONFIG ARRAY via ESP-NOW
		}
		
		// forwarding debugging data UART -> WIFI
		if (length == 6) {
			sendDebuggingData(receive_array[0], receive_array[1], receive_array[2], receive_array[3], receive_array[4], receive_array[5]);
		}
		
		// forwarding flight critical data WIFI -> UART
		if(newWiFidata){
			newWiFidata = false;
			float data[3];
			data[0] = line_length_in_meters;
			data[1] = flight_mode;
			data[2] = groundstation_height;
			sendUARTArray100(data, 3, ESP32_UART_WIFI_PROXY);
		}
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
