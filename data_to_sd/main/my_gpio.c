
//#define GPIO_INPUT_IO_0 26

#define GPIO_INPUT_PIN_SELECTION ((1ULL<<16))

/*
#define GPIO_OUTPUT_PIN_SEL  ((1ULL<<22) | (1ULL<<23))

int get_level_GPIO_0(){
	return gpio_get_level(26);
}

int get_level_GPIO_33(){
	return gpio_get_level(33);
}

int get_level_GPIO_4(){
	return gpio_get_level(4);
}

void set_level_GPIO_22(uint32_t level){
	gpio_set_level(22, level);
}
*/
int get_level_GPIO_16(){
	return gpio_get_level(16);
}

void initGPIO()
{

    gpio_config_t io_conf;
    
    /*
	// OUTPUT
    //disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    //bit mask of the pins that you want to set,e.g.GPIO18/19
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    //disable pull-down mode
    io_conf.pull_down_en = 0;
    //disable pull-up mode
    io_conf.pull_up_en = 0;
    //configure GPIO with the given settings
    gpio_config(&io_conf);
	*/
	
	
    // INPUT
    //gpio_config_t io_conf;
    //interrupt disabled
    io_conf.intr_type = GPIO_INTR_DISABLE;
    //bit mask of the pins
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SELECTION;
    //set as input mode
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_down_en = 0;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
}

