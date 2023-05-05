int bmp280_calibration_timer = -5;
float calibration_pressure = 0;
float calibration_temperature = 0;

uint32_t bmp280_raw_pressure_reading = 0;
uint32_t bmp280_raw_temperature_reading = 0;

float initial_pressure = 0.0;
float initial_temperature = 0.0;

int bmp280_started = -5;
int64_t startTime = 0;

void startPressureMeasurement(){
	// chip_addr, register, precision(0x25 least precise, takes 9 ms, 0x5D most precise, takes 45ms)
	i2c_send(1, 0x76, 0xF4, 0x5D, 1);
}

void readPressureMeasurement(){
	uint8_t highByte = i2c_receive(1, 0x76, 0xF7, 1);
	uint8_t middleByte = i2c_receive(1, 0x76, 0xF8, 1);
	uint8_t lowByte = i2c_receive(1, 0x76, 0xF9, 1);
	bmp280_raw_pressure_reading = (uint32_t)((highByte << 16) | (middleByte << 8) | lowByte);
}

void readTemperatureMeasurement(){
	uint8_t highByte = i2c_receive(1, 0x76, 0xFA, 1);
	uint8_t middleByte = i2c_receive(1, 0x76, 0xFB, 1);
	uint8_t lowByte = i2c_receive(1, 0x76, 0xFC, 1);
	bmp280_raw_temperature_reading = (uint32_t)((highByte << 16) | (middleByte << 8) | lowByte);
}

float getTemperature(){
	return (float)bmp280_raw_temperature_reading;
}

float getPressure(){
	return 1365.3-0.00007555555555*(float)(bmp280_raw_pressure_reading);
}

void initBMP280(){
	startTime = esp_timer_get_time();
}

float time_difference_for_bmp_initialization = 0.0;
float delta_T = 0.0;
float tempSmooth = 0.0;
float getPressureDiff(){
	
	if(bmp280_started == 1){
		tempSmooth = 0.8*tempSmooth + 0.2*getTemperature();
		delta_T = tempSmooth - initial_temperature;
		return getPressure() + MINUS_DP_BY_DT*delta_T - initial_pressure ;
	}else if(bmp280_started >= 2){
	
		// THIS HERE IS ONLY FOR CALIBRATION
		int64_t currentTime = esp_timer_get_time();
		time_difference_for_bmp_initialization = 0.000001*(float)(currentTime - startTime);
		if(time_difference_for_bmp_initialization > 90){					// CALIBRATION after 90 seconds of uptime.
			calibration_pressure += 0.2*getPressure();
			calibration_temperature += 0.2*getTemperature();
			bmp280_started ++;
			if(bmp280_started >= 7){
				printf("calibration pressure: %f, calibration temperature: %f\n", calibration_pressure, calibration_temperature);
				printf("-deltaP/deltaT = %f\n", -(calibration_pressure-initial_pressure)/(calibration_temperature-initial_temperature));
				MINUS_DP_BY_DT_reading = -(calibration_pressure-initial_pressure)/(calibration_temperature-initial_temperature);
				bmp280_started = 1;
			}
		}
		return getPressure() - initial_pressure;
		// END CALIBRATION CODE
		
	}else if(bmp280_started == -5){										// 1: first we're always landing here,
		int64_t currentTime = esp_timer_get_time();
		time_difference_for_bmp_initialization = 0.000001*(float)(currentTime - startTime);
		if(time_difference_for_bmp_initialization > 5){										// 2: until 10 seconds have passed.
			bmp280_started = -4;
			startPressureMeasurement();
		}
	}else{																// 3: then we will land here for 5 times,
		readPressureMeasurement();										// that is: until bmp280_started counter has reached either 1,
		startPressureMeasurement();										// or if CALIBRATION_MODE_BMP280 is on: 2.
		initial_pressure += 0.2*getPressure();
		initial_temperature += 0.2*getTemperature();
		bmp280_started++;
		if(bmp280_started == 1){
			tempSmooth = getTemperature();
			if(CALIBRATION_MODE_BMP280){
				printf("initial pressure: %f, initial temperature: %f\n", initial_pressure, initial_temperature);
				bmp280_started++;
			}
		}
	}
	return 0.0;
}
