#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../helpers/timer.h"
#include "dps310.h"

#define SOURCE_INTERNAL = 0x00
#define SOURCE_EXTERNAL = 0x80

int update_dps310_if_necessary(DPS310 *dps) {
	
	
	uint8_t fifo_empty = i2c_receive(dps->dps310_bus, dps->dps_address, 0x0B, 1);
	while(!(fifo_empty&0x01)){ // FIFO not empty
		dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x00, 1);
		dps->medium_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x01, 1);
		dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x02, 1);
		
		int pressure_or_temperature_reading = dps->high_byte * 65536 + dps->medium_byte * 256 + dps->low_byte;
		if(pressure_or_temperature_reading > 8388608-1) pressure_or_temperature_reading -= 16777216;
		if(pressure_or_temperature_reading & 0x01){
			//printf("pressure reading = %f\n", pressure_or_temperature_reading/3670016.0);
			dps->p_raw = 7.0/8 * dps->p_raw + 1.0/8 * pressure_or_temperature_reading/3670016.0;
		}else{
			//printf("temperature reading = %f\n", pressure_or_temperature_reading/3670016.0);
			dps->t_raw = 7.0/8 * dps->t_raw + 1.0/8 * pressure_or_temperature_reading/3670016.0;
		}
		
		float t_comp = dps->c0*0.5 + dps->c1*dps->t_raw;
		//printf("c0 = %f, c1 = %f, t_raw = %f, t_comp = %f deg C\n", 1.0*c0, 1.0*c1, t_raw, t_comp);
		
		float p_comp = dps->c00 + dps->p_raw*(dps->c10 + dps->p_raw * (dps->c20 + dps->p_raw * dps->c30)) + dps->t_raw * dps->c01 + dps->t_raw * dps->p_raw * (dps->c11 + dps->p_raw * dps->c21);
		
		//printf("p_comp = %f Pa, c00 = %d, c10 = %d, c20 = %d, c30 = %d, c01 = %d, c11 = %d, c21 = %d\n", p_comp, c00, c10, c20, c30, c01, c11, c21);
		
		//printf("p_comp = %f Pa\n", p_comp);
		//printf("height = %f metres\n", -(p_comp-98838)/12);
		if(dps->p_comp_init == 0){
			if(dps->p_init_counter == 80){
				dps->p_comp_init = p_comp;
			}else{
				dps->p_init_counter++;
			}
		}else{
			dps->current_height = -(p_comp-dps->p_comp_init)/12;
		}
		
		fifo_empty = i2c_receive(dps->dps310_bus, dps->dps_address, 0x0B, 1); // FIFO empty?
	}
	//flush FIFO
	//i2c_send(dps310_bus, dps_address, 0x0c, 0x80, 1);
	
	//height_medium_smooth = 0.9* height_medium_smooth + 0.1 * current_height;
	
	float dT = query_timer_seconds(dps->derivativeTimeStep);
	dps->derivativeTimeStep = start_timer();
	if(dT < 0.01) return 0; // DON'T RECALCULATE IF LAST READING IS TOO RECENT / TIME STEP TOO SMALL
	
	dps->dH = (dps->current_height - dps->lastHeight)/dT; // MEDIUM SMOOTHING
	
	dps->lastHeight = dps->current_height;
	
	return 0;
}

void init_dps310(DPS310 *dps, struct i2c_bus bus_arg, int address_arg){ // ToDoLeo rename to init. Make sure init calls in main don't conflict
	
	dps->dps_address = address_arg;
	
	dps->last_update = 0;
	dps->current_height = 0;
	dps->height_medium_smooth = 0;
	dps->p_comp_init = 0;
	dps->p_init_counter = 0;
	dps->coefficientsource = 0;
	dps->p_raw = -0.35135;
	dps->t_raw = 0.287;
	dps->lastHeight = 0;
	dps->derivativeTimeStep = 0;
	dps->dH = 0;
	dps->dps310_bus = bus_arg;
	
	// SOFT RESET
	i2c_send(dps->dps310_bus, dps->dps_address, 0x0c, 0x89, 1);
	
	vTaskDelay(10);
	
	// get temperature source for calibration coefficients
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x28, 1);
	dps->coefficientsource = (dps->high_byte & 0x80);
	
	/*
	if(!coefficientsource)
		printf("temp sensor source = INTERNAL\n");
	else
		printf("temp sensor source = EXTERNAL\n");
	*/
	
	// SET pressure RATE and OVERSAMPLING
	i2c_send(dps->dps310_bus, dps->dps_address, 0x06, 0x50 | 0x02, 1);
	
	// SET temperature RATE and OVERSAMPLING
	i2c_send(dps->dps310_bus, dps->dps_address, 0x07, 0x50 | 0x02 | dps->coefficientsource, 1);
	
	// ENABLE continuous temperature and pressure measurments
	i2c_send(dps->dps310_bus, dps->dps_address, 0x08, 0x07, 1);
	
	// ENABLE FIFO
	i2c_send(dps->dps310_bus, dps->dps_address, 0x09, 0x02, 1);
	
	
	
	// get calibration coefficients
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x10, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x11, 1);
	
	dps->c0 = dps->high_byte*16 + ((dps->low_byte/16)&0x0F);
	if(dps->c0 > (2048-1)) dps->c0 -= 4096;
	
	dps->high_byte = dps->low_byte;
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x12, 1);
	
	dps->c1 = (dps->high_byte&0x0F)*256 + dps->low_byte;
	if(dps->c1 > (2048-1)) dps->c1 -= 4096;
	
	
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x13, 1);
	dps->medium_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x14, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x15, 1);
	
	dps->c00 = dps->high_byte*4096 + dps->medium_byte * 16 + ((dps->low_byte/16)&0x0F);
	if(dps->c00 > (524288-1)) dps->c00 -= 1048576;
	
	dps->high_byte = dps->low_byte;
	dps->medium_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x16, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x17, 1);
	
	dps->c10 = (dps->high_byte&0x0F)*65536 + dps->medium_byte*256 + dps->low_byte;
	if(dps->c10 > (524288-1)) dps->c10 -= 1048576;
	
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x18, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x19, 1);
	
	dps->c01 = dps->high_byte*256 + dps->low_byte;
	if(dps->c01 > (32768-1)) dps->c01 -= 65536;
	
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x1a, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x1b, 1);
	
	dps->c11 = dps->high_byte*256 + dps->low_byte;
	if(dps->c11 > (32768-1)) dps->c11 -= 65536;
	
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x1c, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x1d, 1);
	
	dps->c20 = dps->high_byte*256 + dps->low_byte;
	if(dps->c20 > (32768-1)) dps->c20 -= 65536;
	
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x1e, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x1f, 1);
	
	dps->c21 = dps->high_byte*256 + dps->low_byte;
	if(dps->c21 > (32768-1)) dps->c21 -= 65536;
	
	dps->high_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x20, 1);
	dps->low_byte = i2c_receive(dps->dps310_bus, dps->dps_address, 0x21, 1);
	
	dps->c30 = dps->high_byte*256 + dps->low_byte;
	if(dps->c30 > (32768-1)) dps->c30 -= 65536;
	
	
}

float getHeight(DPS310 *dps) {
	return dps->current_height;
}


// DON'T USE. NEED GROUNDSTATION HEIGHT TO DO THIS.
float getHeightDerivative(DPS310 *dps){
	return dps->dH;
}
