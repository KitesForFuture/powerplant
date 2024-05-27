#include "adc.h"

#define VOLTAGE_ADC_PIN ADC1_CHANNEL_7 // IO35

void initADC(){
    
    //ADC1 config
    ESP_ERROR_CHECK(adc1_config_width(ADC_WIDTH_BIT_DEFAULT));
    ESP_ERROR_CHECK(adc1_config_channel_atten(VOLTAGE_ADC_PIN, ADC_EXAMPLE_ATTEN));
}

int getVoltageInMilliVolt(){
	uint32_t voltage = 0;
	int raw_value = adc1_get_raw(VOLTAGE_ADC_PIN);
	return raw_value;
}
