#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../helpers/timer.h"
#include "dps310_parametrized.h"

#define SOURCE_INTERNAL = 0x00
#define SOURCE_EXTERNAL = 0x80

static uint8_t coefficientsource = 0;

uint8_t high_byte;
uint8_t medium_byte;
uint8_t low_byte;




int update_dps310_if_necessary_p(struct dps310_struct *dps) {
	
	
	uint8_t fifo_empty = i2c_receive(dps->bus, dps->address, 0x0B, 1);
	while(!(fifo_empty&0x01)){ // FIFO not empty
		high_byte = i2c_receive(dps->bus, dps->address, 0x00, 1);
		medium_byte = i2c_receive(dps->bus, dps->address, 0x01, 1);
		low_byte = i2c_receive(dps->bus, dps->address, 0x02, 1);
		
		int pressure_or_temperature_reading = high_byte * 65536 + medium_byte * 256 + low_byte;
		if(pressure_or_temperature_reading > 8388608-1) pressure_or_temperature_reading -= 16777216;
		if(pressure_or_temperature_reading & 0x01){
			//printf("pressure reading = %f\n", pressure_or_temperature_reading/3670016.0);
			dps->p_raw = 7.0/8 * dps->p_raw + 1.0/8 * pressure_or_temperature_reading/3670016.0;
		}else{
			//printf("temperature reading = %f\n", pressure_or_temperature_reading/3670016.0);
			dps->t_raw = 7.0/8 * dps->t_raw + 1.0/8 * pressure_or_temperature_reading/3670016.0;
		}
		
		float t_comp = dps->c0*0.5 + dps->c1*dps->t_raw;
		//printf("c0 = %f, c1 = %f, dps->t_raw = %f, t_comp = %f deg C\n", 1.0*c0, 1.0*c1, dps->t_raw, t_comp);
		
		float p_comp = dps->c00 + dps->p_raw*(dps->c10 + dps->p_raw * (dps->c20 + dps->p_raw * dps->c30)) + dps->t_raw * dps->c01 + dps->t_raw * dps->p_raw * (dps->c11 + dps->p_raw * dps->c21);
		
		
		dps->current_temp = 0.95*dps->current_temp + 0.05*t_comp;
		
		//printf("p_comp = %f Pa, c00 = %d, c10 = %d, c20 = %d, c30 = %d, c01 = %d, c11 = %d, c21 = %d\n", p_comp, c00, c10, c20, c30, c01, c11, c21);
		
		//printf("p_comp = %f Pa\n", p_comp);
		//printf("height = %f metres\n", -(p_comp-98838)/12);
		if(dps->p_comp_init == 0){
			if(dps->p_init_counter == 280){
				dps->p_comp_init = p_comp;
			}else{
				dps->p_init_counter++;
			}
		}else{
			dps->current_height = dps->current_height*0.5 + 0.5*-(p_comp-dps->p_comp_init)/12;
		}
		
		fifo_empty = i2c_receive(dps->bus, dps->address, 0x0B, 1); // FIFO empty?
	}
	//flush FIFO
	//i2c_send(dps->bus, dps->address, 0x0c, 0x80, 1);
	
	//height_medium_smooth = 0.9* height_medium_smooth + 0.1 * current_height;
	
	float dT = query_timer_seconds(dps->derivativeTimeStep);
	dps->derivativeTimeStep = start_timer();
	if(dT < 0.01) return 0; // DON'T RECALCULATE IF LAST READING IS TOO RECENT / TIME STEP TOO SMALL
	
	
	dps->dH = 0.5*dps->dH + 0.5*(dps->current_height - dps->lastHeight)/dT; // MEDIUM SMOOTHING
	
	dps->lastHeight = dps->current_height;
	
	return 0;
}

void init_dps310_p(struct dps310_struct *dps){ // ToDoLeo rename to init. Make sure init calls in main don't conflict
	
	dps->old_dH = 0;
	
	dps->p_raw = -0.35135;
	dps->t_raw = 0.287;
	
    dps->current_height = 0;
    dps->current_temp = 15;
    dps->height_medium_smooth = 0;
    
    dps->p_comp_init = 0;
	dps->p_init_counter = 0;
	
	dps->lastHeight = 0;
	dps->derivativeTimeStep = 0;
	dps->dH = 0;
	
	// SOFT RESET
	i2c_send(dps->bus, dps->address, 0x0c, 0x89, 1);
	
	vTaskDelay(10);
	
	
	// get temperature source for calibration coefficients
	high_byte = i2c_receive(dps->bus, dps->address, 0x28, 1);
	coefficientsource = (high_byte & 0x80);
	
	/*
	if(!coefficientsource)
		printf("temp sensor source = INTERNAL\n");
	else
		printf("temp sensor source = EXTERNAL\n");
	*/
	
	// SET pressure RATE and OVERSAMPLING
	i2c_send(dps->bus, dps->address, 0x06, 0x50 | 0x02, 1);
	
	// SET temperature RATE and OVERSAMPLING
	i2c_send(dps->bus, dps->address, 0x07, 0x50 | 0x02 | coefficientsource, 1);
	
	// ENABLE continuous temperature and pressure measurments
	i2c_send(dps->bus, dps->address, 0x08, 0x07, 1);
	
	// ENABLE FIFO
	i2c_send(dps->bus, dps->address, 0x09, 0x02, 1);
	
	
	
	// get calibration coefficients
	high_byte = i2c_receive(dps->bus, dps->address, 0x10, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x11, 1);
	
	dps->c0 = high_byte*16 + ((low_byte/16)&0x0F);
	if(dps->c0 > (2048-1)) dps->c0 -= 4096;
	
	high_byte = low_byte;
	low_byte = i2c_receive(dps->bus, dps->address, 0x12, 1);
	
	dps->c1 = (high_byte&0x0F)*256 + low_byte;
	if(dps->c1 > (2048-1)) dps->c1 -= 4096;
	
	
	high_byte = i2c_receive(dps->bus, dps->address, 0x13, 1);
	medium_byte = i2c_receive(dps->bus, dps->address, 0x14, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x15, 1);
	
	dps->c00 = high_byte*4096 + medium_byte * 16 + ((low_byte/16)&0x0F);
	if(dps->c00 > (524288-1)) dps->c00 -= 1048576;
	
	high_byte = low_byte;
	medium_byte = i2c_receive(dps->bus, dps->address, 0x16, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x17, 1);
	
	dps->c10 = (high_byte&0x0F)*65536 + medium_byte*256 + low_byte;
	if(dps->c10 > (524288-1)) dps->c10 -= 1048576;
	
	high_byte = i2c_receive(dps->bus, dps->address, 0x18, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x19, 1);
	
	dps->c01 = high_byte*256 + low_byte;
	if(dps->c01 > (32768-1)) dps->c01 -= 65536;
	
	high_byte = i2c_receive(dps->bus, dps->address, 0x1a, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x1b, 1);
	
	dps->c11 = high_byte*256 + low_byte;
	if(dps->c11 > (32768-1)) dps->c11 -= 65536;
	
	high_byte = i2c_receive(dps->bus, dps->address, 0x1c, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x1d, 1);
	
	dps->c20 = high_byte*256 + low_byte;
	if(dps->c20 > (32768-1)) dps->c20 -= 65536;
	
	high_byte = i2c_receive(dps->bus, dps->address, 0x1e, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x1f, 1);
	
	dps->c21 = high_byte*256 + low_byte;
	if(dps->c21 > (32768-1)) dps->c21 -= 65536;
	
	high_byte = i2c_receive(dps->bus, dps->address, 0x20, 1);
	low_byte = i2c_receive(dps->bus, dps->address, 0x21, 1);
	
	dps->c30 = high_byte*256 + low_byte;
	if(dps->c30 > (32768-1)) dps->c30 -= 65536;
	
	
}

float getHeight_p(struct dps310_struct* dps) {
	return dps->current_height;
}


// DON'T USE. NEED GROUNDSTATION HEIGHT TO DO THIS.
float getHeightDerivative_p(struct dps310_struct* dps){
	return dps->dH;
}


float getTemp_p(struct dps310_struct *dps){
	return dps->current_temp;
}

