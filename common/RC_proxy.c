#include "num_config_vars.h"
#include "uart.c"

#define RC_MODE 0
#define DATA_MODE 1
#define LINE_TENSION_REQUEST_MODE 2
#define LINE_LENGTH_MODE 3
#define CONFIG_MODE 4
#define DEBUG_DATA_MODE 5

#define DATALENGTH 3


#define ESP32_UART_WIFI_PROXY 1


static uint8_t broadcast_mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const uint8_t WIFI_CHANNEL = 0;

float line_length_in_meters = 1;
float flight_mode = 0;
float groundstation_height = 0;

//float tension_request = 0;

static void (*write_config_callback_kite)(float*);

// init uart on the esp
void network_setup_kite_flying(void (*write_config_callback_arg)(float*))
{
    // uart to Wifi-ESP32
	initUART(ESP32_UART_WIFI_PROXY, GPIO_NUM_22, GPIO_NUM_26, false);
    
	write_config_callback_kite = write_config_callback_arg;
	
}

void sendDebuggingData(float num1, float num2, float num3, float num4, float num5, float num6){
	float data[6];
	data[0] = num1;
	data[1] = num2;
	data[2] = num3;
	data[3] = num4;
	data[4] = num5;
	data[5] = num6;
	
	sendUARTArray100(data, 6, ESP32_UART_WIFI_PROXY);
}

void sendDataArrayLarge(uint32_t mode, float* data, int length){
	sendUARTArray100(data, length, ESP32_UART_WIFI_PROXY);
}

static int length = 0;
static float receive_array[200];

void processRC(){
	//check for uart data
	length = processUART(ESP32_UART_WIFI_PROXY, receive_array);
	
	// config data
	if (length == NUM_CONFIG_FLOAT_VARS) {
		(*write_config_callback_kite)(receive_array);
	}
	
	// line_length, flight_mode, groundstation_height
	if (length == 3){
		line_length_in_meters = receive_array[0];
		flight_mode = receive_array[1];
		groundstation_height = receive_array[2];
	}
}

