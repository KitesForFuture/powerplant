
#include "pwm_input.h"

#define NUM_PWM_INPUT_CHANNELS 6

int num_registered_channels = 0;
int pwm_input_channels[NUM_PWM_INPUT_CHANNELS] = {1, 2, 3, 4, 5, 6};
int pwm_in_gpios[NUM_PWM_INPUT_CHANNELS] = {26, 27, 12, 13, 0, 2};//{2, 0, 4, 5};

float smoothedSWC = 0;

#define PWM_INPUT_MIN_DUTY 8000
#define PWM_INPUT_MAX_DUTY 16000

RingbufHandle_t pwm_buffer_handles[NUM_PWM_INPUT_CHANNELS] = {NULL};

int pwm_input_value[NUM_PWM_INPUT_CHANNELS] = {PWM_INPUT_MIN_DUTY, PWM_INPUT_MIN_DUTY, PWM_INPUT_MIN_DUTY, PWM_INPUT_MIN_DUTY};

float getPWMInput0to1normalized(int num){
	float ret = ((float)(pwm_input_value[num] - PWM_INPUT_MIN_DUTY))/(PWM_INPUT_MAX_DUTY - PWM_INPUT_MIN_DUTY);
	if(ret > 1) ret = 1;
	if(ret < 0) ret = 0;
	return ret;
}

float getPWMInputMinus1to1normalized(int num){
	return 2*getPWMInput0to1normalized(num)-1;
}

void initPWMInput(int pins[], int length){
	
	for(int i = 0; (i < length && i < NUM_PWM_INPUT_CHANNELS); i++){
		pwm_in_gpios[i] = pins[i];
		num_registered_channels++;
	}
	/*
	pwm_in_gpios[0] = pin0;
	pwm_in_gpios[1] = pin1;
	pwm_in_gpios[2] = pin2;
	pwm_in_gpios[3] = pin3;
	*/
	rmt_config_t rmt_channel; // = RMT_DEFAULT_CONFIG_RX(2, 1);
	
	for(int i = 0; i < num_registered_channels; i++){
		rmt_channel.channel = (rmt_channel_t) pwm_input_channels[i];
		rmt_channel.gpio_num = (gpio_num_t) pwm_in_gpios[i];
		rmt_channel.clk_div = 10;//(80000000/8/1000000); // = 10, 80 causes kernel panic, 5 causes doubling of re/per/ceived pulse widths but not able to handle 2*16000=32000, uint8t is in [0, 255] so why does 80 not work?
		rmt_channel.mem_block_num = 1;
		rmt_channel.rmt_mode = RMT_MODE_RX; // RX means receive (TX would mean transceive=emit)
		rmt_channel.rx_config.filter_en = true;
		rmt_channel.rx_config.filter_ticks_thresh = 100; // TODO: what do these mean?
		rmt_channel.rx_config.idle_threshold = 3500 * 8; // TODO: what do these mean?
		
		rmt_config(&rmt_channel);
		
		rmt_driver_install(rmt_channel.channel, (size_t)100, 0);
		
		rmt_get_ringbuf_handle(rmt_channel.channel, &(pwm_buffer_handles[i]));
		
		rmt_rx_start(rmt_channel.channel, 1); // 1 = boolean true
	}
}



void updatePWMInput(){
	
	for(int i = 0; i < num_registered_channels; i++){
		
		while(1){
			// retrieve item from ring buffer BY REFERENCE(!)
			// xRingbufferReceive BLOCKS !!!
			uint32_t length = 0;
			
			rmt_item32_t *items  = (rmt_item32_t *) xRingbufferReceive(pwm_buffer_handles[i], &length/*will be 4 (meaning 4 bytes, i.e. 32 bits)*/, 1/*if too high buffer runs full for long pulses?*/);
			if (items) {
			   	// items->duration0 is between 8000 and 16000, further members of "items" are: items->level0, items->duration1, items->level1
			   	pwm_input_value[i] = items->duration0;
			   	// "RETURNING" the REFERENCE to the ring buffer, so it feels confident to overwrite it when neccessary.
				vRingbufferReturnItem(pwm_buffer_handles[i], (void *) items);
			}else{
				break;
			}
		}
	}
}
