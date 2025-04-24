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

#include "../../common/i2c_devices/dps310.h"

#include "../../common/uart.c"

#include "motors.c"

#include "my_gpio.c"

#include "../../common/RC.c"

#include "driver/ledc.h"


#include "esp_system.h"
#include "driver/spi_master.h"
#include "lora_minimum.h"
#include "lora_minimum.c"

#define SERVO_MIN_ANGLE -45
#define SERVO_MAX_ANGLE 54

#define NOT_INITIALIZED -1000000

#define VESC_UART 0
#define ESP32_UART 1

#define LAND_COMMAND 0
#define LAUNCH_COMMAND 1

#define LEDC_TIMER              LEDC_TIMER_0
#define LEDC_MODE               LEDC_LOW_SPEED_MODE
#define LEDC_OUTPUT_IO          (33) // Define the output GPIO
#define LEDC_CHANNEL            LEDC_CHANNEL_0
#define LEDC_DUTY_RES           LEDC_TIMER_13_BIT // Set duty resolution to 13 bits
#define LEDC_DUTY               (4096) // Set duty to 50%. (2 ** 13) * 50% = 4096
#define LEDC_FREQUENCY          (4000) // Frequency in Hertz. Set frequency at 4 kHz

struct i2c_bus bus0 = {25, 14};//SDA, SCL

int internet_connected = false;

float currentServoAngle = SERVO_MIN_ANGLE;
float direction = 1;

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

float wifi_send_led_state = 0;
float wifi_rec_led_state = 0;
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
		config_values_kite_and_gs[NUM_CONFIG_FLOAT_VARS + 0] = 0; // not needed anymore
		sendUARTArray100(config_values_kite_and_gs, NUM_CONFIG_FLOAT_VARS + NUM_GS_CONFIG_FLOAT_VARS, ESP32_UART); // FORWARD config (appending groundstation config) to access point ESP32
	}
}


float data_including_lora[10];

void processReceivedDebuggingDataViaWiFi(float* debugging_data, int length){
	//printf("forwarding Debugging data to in_flight_config ESP32");
	//debugging_data[1] = getHeight();
	/*if(wifi_rec_led_state == 0){
		//set_level_GPIO_22(1);
		wifi_rec_led_state = 1;
	}else{
		set_level_GPIO_22(0);
		wifi_rec_led_state = 0;
	}*/
	for(int i = 0; i < lenght-1; i++){
		data_including_lora[i] = debugging_data[i];
	}
	//fill the last 4 values with groundstation and lora stuff
	
	data_including_lora[9] = debugging_data[length-1]; // flight mode indicator
	
}

void init(){
	dps_address = 0x77;
    init_dps310(bus0);
	
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
	
	//example_ledc_init();
	//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
	//ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
	
	
	init_uptime();
	
	lora_init();
	lora_implicit_header_mode(2);
	lora_enable_crc();
	lora_set_bandwidth(7);
	lora_set_spreading_factor(6);
	
	for(int i = 0; i < 10; i++){
		data_including_lora[i] = 0;
	}
	
}

//float line_speed = 0;
float line_speed_servo = 0;
float line_length_offset = NOT_INITIALIZED;
float last_line_length = 0;

float lora_received_1_old = 0;

float old_line_length = 0;

void app_main(void){
	init();
	//storeServoArmForEnergyGeneration();
	//controlServoAngle(30.0 * reel_in_high_duty_voltage);
	//float landing_request = 0.0;
	float receive_array[100];
	int receive_array_length = 0;
	float line_length_raw = 0;
	float old_line_length_raw = 0;
	float old_line_length_raw2 = 0;
	float flight_mode = 0;
	printf("waiting for UART...\n");
	uint8_t counter = 0;
	//float line_length_test = 0;
	float line_length = 0;
	float lora_received_1 = 0;
	float lora_received_2 = 0;
	
	float debugging_line_length = 0;
	
	int line_length_was_over_60 = false;
	
	set_level_GPIO_22(0);
	set_level_GPIO_23(0);
	
	while(1){
		
		debugging_line_length += 0.1;
		//line_length = debugging_line_length;
		
		counter++;
		uint8_t buf[4];
		if(counter%4 == 0){
			/*if(line_length_test < 2000)
				line_length_test ++;
			else
				line_length_test = 0;*/
			int ll_times_16 = (int)(clamp(line_length, 0, 4000) * 16);
			buf[0] = (ll_times_16 >> 8) & 0xFF;
			buf[1] = ll_times_16 & 0xFF;
			
			int gsho_times_4 = ((int)((getHeight()+0.5) * 32)) % 32;
			if(flight_mode == 112){ flight_mode = 7;}
			buf[2] = (((char)flight_mode)<<5) + (gsho_times_4 & 0x1F);
			
			int ls_times_8 = (int)(clamp(-line_speed, 0, 30) * 8);
			buf[3] = ls_times_8 & 0xFF;
			
			printf("dll = %f, sending fm=%d, line_length=%f, line_speed=%f to kite [%d, %d, %d, %d]\n", debugging_line_length, (char)flight_mode, line_length, line_speed, buf[0], buf[1], buf[2], buf[3]);
			lora_send_packet_and_forget(buf, 4);
			lora_receive(4);
			
			
			//send this regardless of whether data comes in via LoRa or WIFI
			data_including_lora[5] = line_length;
			data_including_lora[6] = getHeight();
			
			
			data_including_lora[7] = lora_received_1; // == line_length, maybe with delay or freezing when packet lost
			data_including_lora[8] = lora_received_2;
			
			/*if(abs(lora_received_1 - lora_received_1_old) < 10 && (lora_received_2 == -5 || lora_received_2 == 0 || lora_received_2 == 1 || lora_received_2 == 2 || lora_received_2 == 4 || lora_received_2 == 6 || lora_received_2 == 7 || lora_received_2 == 112)){
				
			}else{
				if(lora_received_1 > 3){
					data_including_lora[7] = -10; // == line_length, maybe with delay or freezing when packet lost
					data_including_lora[8] = -10;
				}
			}
			*/
			if(abs(lora_received_1 - lora_received_1_old) < 1){
				set_level_GPIO_23(1);
			}
			lora_received_1_old = lora_received_1;
			
			sendUARTArray100(data_including_lora, 10, ESP32_UART);
		}
		if(lora_received()){
			int return_value = lora_receive_packet(buf, 4);
			if(return_value != 0){
				lora_received_1 = ((((int)buf[0]) << 8) + buf[1]) / 16.0;
				lora_received_2 = buf[2];
				if(buf[2] == 4){//autopilot.mode == LANDING_MODE
					tension_request = 1.0; // kite is in landing mode
				}
				printf("received [%d, %d, %d, %d], lr1 = %f, lr2 = %f, ret = %d\n", buf[0], buf[1], buf[2], buf[3], lora_received_1, lora_received_2, return_value);
			}
		}
		
		
		
		/*
		if(((int)(get_uptime_seconds()))%2 == 0){
			set_level_GPIO_22(0);
			set_level_GPIO_23(1);
		}else{
			set_level_GPIO_22(1);
			set_level_GPIO_23(0);
		}
		*/
		
		//printf("test314\n");
		update_dps310_if_necessary();
		//printf("getHeight() = %f\n", getHeight());
		//sendUART(1, 2, VESC_UART);//DEBUGGING
		//sendUART(1, 2, ESP32_UART);//DEBUGGING
		// **************** REACT to UART message from VESC ****************
		
		// pass line length and line tension from UART to WIFI
		receive_array_length = processUART(VESC_UART, receive_array);
		if(receive_array_length == 3){
			
			//printf("received UART from VESC\n");
			if(led_state == 0){
				//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
				led_state = 1;
			}else{
				//ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
				led_state = 0;
			}
			//ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
			
			line_length_raw = receive_array[0];
			flight_mode = receive_array[1];
			//printf("received flight_mode_request = %f\n", flight_mode);
			line_speed = receive_array[2];
			if(flight_mode != 2.0 && flight_mode != 3.0){tension_request = 0;};
			
			// REMOVING UP TO 2 CONSECUTIVE OUTLIERS of LINE_LENGTH
			if( (line_length_raw < old_line_length_raw + 3) && (line_length_raw > old_line_length_raw - 3) && (line_length_raw < old_line_length_raw2 + 3) && (line_length_raw > old_line_length_raw2 - 3)){
				line_length = line_length_raw;// - line_length_offset;
			}
			old_line_length_raw2 = old_line_length_raw;
			old_line_length_raw = line_length_raw;
			
			//for finding line length bug
			if (old_line_length + 3 < line_length || old_line_length - 3 > line_length){//if line-length jumps by more than 10 metres
            	set_level_GPIO_22(1);
            }
            old_line_length = line_length;
            
			//printf("line_length = %f\n", line_length);
			//printf("received %f, %f\n", line_length_raw, flight_mode);
			//printf("sending flight_mode %f and line_length %f to kite\n", flight_mode, line_length);
			
			/*if(wifi_send_led_state == 0){
				//set_level_GPIO_23(1);
				wifi_send_led_state = 1;
			}else{
				set_level_GPIO_23(0);
				wifi_send_led_state = 0;
			}*/
			
			/*
			sendData(LINE_LENGTH_MODE, line_length, flight_mode, getHeight(), line_speed); // send line_length, flight_mode to kite
			printf("sending flight_mode %f and line_length %f to communication ESP32\n", flight_mode, line_length);
			*/
			
			if(internet_connected){
				sendUART(flight_mode, line_length, ESP32_UART); // send flight_mode to attached ESP32, which forwards it to the internet
			}
			line_speed_servo = (1-0.125) * line_speed_servo + 0.125 * 50/*frequency*/ * (line_length - last_line_length);
			last_line_length = line_length;
			//printf("line_speed = %f, line_length = %f\n", line_speed, line_length);
			if(-line_speed_servo > 0.5){
				controlServoAngle(30.0 * fabs(line_speed_servo));
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
			//set_bmp_calibration(receive_array[NUM_CONFIG_FLOAT_VARS+0]); // we have no BMP anymore that needs calibrating
		}
		
		// **************** MANUAL SWITCH ****************
		
		if(line_length > 60){
			line_length_was_over_60 = true;
		}
		
		if(!internet_connected){
			if(get_level_GPIO_0() != 0 || (line_length_was_over_60 == true && line_length < 50)){
				printf("SWITCH request final landing\n");
				sendUART(2, 0, VESC_UART); // request final-landing from VESC
			}else if(get_level_GPIO_33() != 0){
				//Switch request for emergency landing (in circles, airbrake extended, line tension = 0)
				printf("SWITCH request emergency landing\n");
				sendUART(112, 0, VESC_UART); // request emergency-landing from VESC
			}else{
				//sendUART(0, 0, VESC_UART);
			}
		}
		
	    if(tension_request == 1.0){		// kite is in landing mode, thus ...
	    	sendUART(1, 0, VESC_UART);	// ... agree to landing with VESC
	    }
	    vTaskDelay(2.0);
    }
    
}
