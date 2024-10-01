
#include <math.h>
//#include <stdio.h>
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

#include "RC_for_in_flight_config.c"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "../../common/uart.c"
#include "timer.c"

#include "driver/ledc.h"

#include "my_gpio.c"

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (33) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz


#define ESP32_UART 0

float led_state = 0;
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
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LEDC_OUTPUT_IO,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

float config_values[NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS];

int data_needs_being_send_via_UART = 0;

float debuggingData[6];
int debuggingDataInitialized = false;

void getDebuggingData(float* values){
	//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
	//ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
	for(int i = 0; i < 6; i++){
		values[i] = debuggingDataInitialized? debuggingData[i] : 1000000;
	}
	//vTaskDelay(1);
	//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
	//ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
	/*if(led_state == 0){
		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
		led_state = 1;
	}else{
		ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
		led_state = 0;
	}*/
	//ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void getConfigValues(float* values){
	for (int i = 0; i < NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS; i++){
		values[i] = config_values[i];
	}
}

void setConfigValues(float* values){
	for (int i = 6; i < NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS; i++){
		config_values[i] = values[i];
	}
	data_needs_being_send_via_UART = 1;
}

void app_main(void)
{
	//network_setup_configuring(&setConfigValues, &getConfigValues, &getDebuggingData);//FOR DEBUGGING ONLY !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//printf("network setup finished\n");
	initGPIO();
	
	example_ledc_init();
	ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
	ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
	
	
	initUART(ESP32_UART, GPIO_NUM_17, GPIO_NUM_16, false);
	//initializeConfigValues(config_values);
	
	int length = 0;
	float receive_array[200];
	int network_initialized = false;
	//printf("waiting for config data from kite...\n");
	vTaskDelay(100);
	//int64_t starttime = esp_timer_get_time();
	while(1){
		if(((int)(get_uptime_seconds()))%2 == 0){
			set_level_GPIO_22(0);
			set_level_GPIO_23(1);
		}else{
			set_level_GPIO_22(1);
			set_level_GPIO_23(0);
		}
		
		/*int64_t current_time = esp_timer_get_time();
		float timeSinceBoot = 0.000001*(float)(current_time - starttime);
		if(!network_initialized && timeSinceBoot > 10){
			esp_restart();
		}*/
		//printf("looping\n");
		//sendUART(1,2, ESP32_UART);//DEBUGGING
		//sendUARTArray100(config_values, NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS, ESP32_UART);//DEBUGGING
		if(data_needs_being_send_via_UART == 1){
			sendUARTArray100(config_values, NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS, ESP32_UART);
			data_needs_being_send_via_UART = 0;
		}
		length = processUART(ESP32_UART, receive_array);
		
		// *** ECHO config array and INITIALIZE values[] ***
		if (length == NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS) {
			printf("Received config from kite (and groundstation: ");
			for(int i = 6; i < NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS; i++){
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
			sendUARTArray100(config_values, NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS, ESP32_UART);
		}else if (length == 2){
			//ignore, this is status message for internet
		}else if (length == 6){
			for(int i = 0; i < 6; i++) debuggingData[i] = receive_array[i];
			debuggingDataInitialized = true;
			//printf("received debugging data\n");
		}
		
		vTaskDelay(10);
	}
}
