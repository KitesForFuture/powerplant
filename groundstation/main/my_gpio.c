
#define GPIO_INPUT_IO_0 26

#define GPIO_INPUT_PIN_SELECTION ((1ULL<<GPIO_INPUT_IO_0))

int get_level_GPIO_0(){
	return gpio_get_level(GPIO_INPUT_IO_0);
}

void initGPIO()
{
    // INPUT
    gpio_config_t io_conf;
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

