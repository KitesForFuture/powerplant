#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../helpers/timer.h"
#include "dps310.h"

#define SOURCE_INTERNAL = 0x00
#define SOURCE_EXTERNAL = 0x80

struct i2c_bus dps310_bus;
static Time last_update = 0;

int dps_address = 0x77;

float current_height = 0;
float height_medium_smooth = 0;

float p_comp_init = 0;
int p_init_counter = 0;

uint8_t coefficientsource = 0;

int c0, c1;

int c00, c10, c01, c11, c20, c21, c30;

uint8_t high_byte;
uint8_t medium_byte;
uint8_t low_byte;

float p_raw = -0.35135;
float t_raw = 0.287;

static float lastHeight = 0;
static Time derivativeTimeStep = 0;
static float dH = 0;

int update_dps310_if_necessary() {
	
	
	uint8_t fifo_empty = i2c_receive(dps310_bus, dps_address, 0x0B, 1);
	while(!(fifo_empty&0x01)){ // FIFO not empty
		high_byte = i2c_receive(dps310_bus, dps_address, 0x00, 1);
		medium_byte = i2c_receive(dps310_bus, dps_address, 0x01, 1);
		low_byte = i2c_receive(dps310_bus, dps_address, 0x02, 1);
		
		int pressure_or_temperature_reading = high_byte * 65536 + medium_byte * 256 + low_byte;
		if(pressure_or_temperature_reading > 8388608-1) pressure_or_temperature_reading -= 16777216;
		if(pressure_or_temperature_reading & 0x01){
			//printf("pressure reading = %f\n", pressure_or_temperature_reading/3670016.0);
			p_raw = 7.0/8 * p_raw + 1.0/8 * pressure_or_temperature_reading/3670016.0;
		}else{
			//printf("temperature reading = %f\n", pressure_or_temperature_reading/3670016.0);
			t_raw = 7.0/8 * t_raw + 1.0/8 * pressure_or_temperature_reading/3670016.0;
		}
		
		float t_comp = c0*0.5 + c1*t_raw;
		//printf("c0 = %f, c1 = %f, t_raw = %f, t_comp = %f deg C\n", 1.0*c0, 1.0*c1, t_raw, t_comp);
		
		float p_comp = c00 + p_raw*(c10 + p_raw * (c20 + p_raw * c30)) + t_raw * c01 + t_raw * p_raw * (c11 + p_raw * c21);
		
		//printf("p_comp = %f Pa, c00 = %d, c10 = %d, c20 = %d, c30 = %d, c01 = %d, c11 = %d, c21 = %d\n", p_comp, c00, c10, c20, c30, c01, c11, c21);
		
		//printf("p_comp = %f Pa\n", p_comp);
		//printf("height = %f metres\n", -(p_comp-98838)/12);
		if(p_comp_init == 0){
			if(p_init_counter == 80){
				p_comp_init = p_comp;
			}else{
				p_init_counter++;
			}
		}else{
			current_height = -(p_comp-p_comp_init)/12;
		}
		
		fifo_empty = i2c_receive(dps310_bus, dps_address, 0x0B, 1); // FIFO empty?
	}
	//flush FIFO
	//i2c_send(dps310_bus, dps_address, 0x0c, 0x80, 1);
	
	//height_medium_smooth = 0.9* height_medium_smooth + 0.1 * current_height;
	
	float dT = query_timer_seconds(derivativeTimeStep);
	derivativeTimeStep = start_timer();
	if(dT < 0.01) return 0; // DON'T RECALCULATE IF LAST READING IS TOO RECENT / TIME STEP TOO SMALL
	
	dH = (current_height - lastHeight)/dT; // MEDIUM SMOOTHING
	
	lastHeight = current_height;
	
	return 0;
}

void init_dps310(struct i2c_bus bus_arg){ // ToDoLeo rename to init. Make sure init calls in main don't conflict
	dps310_bus = bus_arg;
	
	// SOFT RESET
	i2c_send(dps310_bus, dps_address, 0x0c, 0x89, 1);
	
	vTaskDelay(10);
	
	
	// get temperature source for calibration coefficients
	high_byte = i2c_receive(dps310_bus, dps_address, 0x28, 1);
	coefficientsource = (high_byte & 0x80);
	
	/*
	if(!coefficientsource)
		printf("temp sensor source = INTERNAL\n");
	else
		printf("temp sensor source = EXTERNAL\n");
	*/
	
	// SET pressure RATE and OVERSAMPLING
	i2c_send(dps310_bus, dps_address, 0x06, 0x50 | 0x02, 1);
	
	// SET temperature RATE and OVERSAMPLING
	i2c_send(dps310_bus, dps_address, 0x07, 0x50 | 0x02 | coefficientsource, 1);
	
	// ENABLE continuous temperature and pressure measurments
	i2c_send(dps310_bus, dps_address, 0x08, 0x07, 1);
	
	// ENABLE FIFO
	i2c_send(dps310_bus, dps_address, 0x09, 0x02, 1);
	
	
	
	// get calibration coefficients
	high_byte = i2c_receive(dps310_bus, dps_address, 0x10, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x11, 1);
	
	c0 = high_byte*16 + ((low_byte/16)&0x0F);
	if(c0 > (2048-1)) c0 -= 4096;
	
	high_byte = low_byte;
	low_byte = i2c_receive(dps310_bus, dps_address, 0x12, 1);
	
	c1 = (high_byte&0x0F)*256 + low_byte;
	if(c1 > (2048-1)) c1 -= 4096;
	
	
	high_byte = i2c_receive(dps310_bus, dps_address, 0x13, 1);
	medium_byte = i2c_receive(dps310_bus, dps_address, 0x14, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x15, 1);
	
	c00 = high_byte*4096 + medium_byte * 16 + ((low_byte/16)&0x0F);
	if(c00 > (524288-1)) c00 -= 1048576;
	
	high_byte = low_byte;
	medium_byte = i2c_receive(dps310_bus, dps_address, 0x16, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x17, 1);
	
	c10 = (high_byte&0x0F)*65536 + medium_byte*256 + low_byte;
	if(c10 > (524288-1)) c10 -= 1048576;
	
	high_byte = i2c_receive(dps310_bus, dps_address, 0x18, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x19, 1);
	
	c01 = high_byte*256 + low_byte;
	if(c01 > (32768-1)) c01 -= 65536;
	
	high_byte = i2c_receive(dps310_bus, dps_address, 0x1a, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x1b, 1);
	
	c11 = high_byte*256 + low_byte;
	if(c11 > (32768-1)) c11 -= 65536;
	
	high_byte = i2c_receive(dps310_bus, dps_address, 0x1c, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x1d, 1);
	
	c20 = high_byte*256 + low_byte;
	if(c20 > (32768-1)) c20 -= 65536;
	
	high_byte = i2c_receive(dps310_bus, dps_address, 0x1e, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x1f, 1);
	
	c21 = high_byte*256 + low_byte;
	if(c21 > (32768-1)) c21 -= 65536;
	
	high_byte = i2c_receive(dps310_bus, dps_address, 0x20, 1);
	low_byte = i2c_receive(dps310_bus, dps_address, 0x21, 1);
	
	c30 = high_byte*256 + low_byte;
	if(c30 > (32768-1)) c30 -= 65536;
	
	
}

float getHeight() {
	return current_height;
}


// DON'T USE. NEED GROUNDSTATION HEIGHT TO DO THIS.
float getHeightDerivative(){
	return dH;
}
