/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <math.h>
#include "internetStation.c"

#include "driver/gpio.h"
#include "driver/uart.h"
#include "uart.c"

#define ESP32_UART 0

void app_main(void)
{
    init_internet_connection();
    initUART(ESP32_UART, GPIO_NUM_19, GPIO_NUM_18, false);
    //initUART(UART_NUM_2, GPIO_NUM_18, GPIO_NUM_19);
    
    float received_data[100];
    int length_of_received_data = 0;
    
	while(1){
    	sendUART(current_command, 0, ESP32_UART);
		length_of_received_data = processUART(ESP32_UART, received_data);
		if(length_of_received_data >= 2){
			current_status = received_data[0];
			current_line_length = received_data[1];
		}
		vTaskDelay(10);
	}
}
