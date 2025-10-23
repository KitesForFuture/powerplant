
static esp_adc_cal_characteristics_t *adc_chars;

// ADC (ANALOG 2 DIGITAL CONVERTER)
void initADC(){
	//Configure ADC (ANALOG 2 DIGITAL CONVERTER)
	adc1_config_width(ADC_WIDTH_BIT_12);
	//Characterize ADC
    adc_chars = calloc(1, sizeof(esp_adc_cal_characteristics_t));
    //set the linear ANALOG TO DIGITAL conversion function, 1100 means 1.1 Volts
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3900 , adc_chars);
    
    //there is ADC_UNIT_1 controlling the pins G32, G33, G34, G35, SP and SN.
    //there is also ADC_UNIT_2 for more input, which supposedly works only with wifi turned off.
}

void initSensors(){
	initADC();
	
	//ADC_CHANNEL_X is hard wired to a GPIO on the board.
	
	adc1_config_channel_atten(ADC_CHANNEL_0, ADC_ATTEN_DB_11);//SP
	adc1_config_channel_atten(ADC_CHANNEL_3, ADC_ATTEN_DB_11);//SN
	adc1_config_channel_atten(ADC_CHANNEL_6, ADC_ATTEN_DB_11);//G34
	adc1_config_channel_atten(ADC_CHANNEL_7, ADC_ATTEN_DB_11);//G35
	adc1_config_channel_atten(ADC_CHANNEL_4, ADC_ATTEN_DB_11);//G32
	adc1_config_channel_atten(ADC_CHANNEL_5, ADC_ATTEN_DB_11);//G33
	
}

uint32_t adc_reading = 0;

uint32_t getSensor(int no){
	adc1_channel_t cha = ADC_CHANNEL_0;
	switch(no) {
		case 0: cha = (adc1_channel_t)ADC_CHANNEL_0; break;
		case 1: cha = (adc1_channel_t)ADC_CHANNEL_3; break;
		case 2: cha = (adc1_channel_t)ADC_CHANNEL_7; break;
		case 3: cha = (adc1_channel_t)ADC_CHANNEL_4; break;
		case 4: cha = (adc1_channel_t)ADC_CHANNEL_5; break;
		case 5: cha = (adc1_channel_t)ADC_CHANNEL_6; break;
		default: cha = (adc1_channel_t)ADC_CHANNEL_0; break;
	}
	//read ANALOG 2 DIGITAL pin (e.g. a potentiometer), output between 1700 and 4095 (looks like 2^12)
	adc_reading = adc1_get_raw(cha);
	//printf("raw a2d reading is %d", adc_reading);
	uint32_t voltage = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
	//printf("Raw: %d\tVoltage: %dmV\n", adc_reading, voltage);
	//convert digitalized signal to voltage
	return voltage;
}

