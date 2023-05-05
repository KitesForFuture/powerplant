

/**
 * GPIO status:
 * GPIO2:  input, pulled up, interrupt from rising edge and falling edge
 */

#define RUDDER_CHANNEL     0
#define ELEVATOR_CHANNEL     2
#define THRUST_CHANNEL     4
#define POTI_CHANNEL     5
#define SWITCH_SWC_CHANNEL     15
#define GPIO_INPUT_PIN_SEL  1ULL<<ELEVATOR_CHANNEL
//#define GPIO_INPUT_PIN_SEL  ((1ULL<<RUDDER_CHANNEL) | (1ULL<<ELEVATOR_CHANNEL) | (1ULL<<THRUST_CHANNEL) | (1ULL<<POTI_CHANNEL) | (1ULL<<SWITCH_SWC_CHANNEL))
#define ESP_INTR_FLAG_DEFAULT 0

static xQueueHandle gpio_evt_queue = NULL;

struct pwm_data{
	float rise_to_rise_time;
	int64_t last_rise_time;
	float duty;
};

struct pwm_data rudder_pwm_data = {1, 0, 1};
struct pwm_data elevator_pwm_data = {1, 0, 1};
struct pwm_data thrust_pwm_data = {1, 0, 1};
struct pwm_data poti_pwm_data = {1, 0, 1};
struct pwm_data switch_swc_pwm_data = {1, 0, 1};


static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    xQueueSendFromISR(gpio_evt_queue, &gpio_num, NULL);
}

static void gpio_task_example(void* arg)
{
	uint32_t io_num;
    for(;;) {
        if(xQueueReceive(gpio_evt_queue, &io_num, portMAX_DELAY)) {
            // io_num = RUDDER_CHANNEL or ELEVATOR_CHANNEL, ...
            struct pwm_data *pointer_to_pwm_data = &elevator_pwm_data;
			/*switch(io_num) {
				case RUDDER_CHANNEL  :
					pointer_to_pwm_data = &rudder_pwm_data;
					break;
				case ELEVATOR_CHANNEL  :
					pointer_to_pwm_data = &elevator_pwm_data;
					break;
				case THRUST_CHANNEL  :
					pointer_to_pwm_data = &thrust_pwm_data;
					break;
				case POTI_CHANNEL  :
					pointer_to_pwm_data = &poti_pwm_data;
					break;
				case SWITCH_SWC_CHANNEL  :
					pointer_to_pwm_data = &switch_swc_pwm_data;
					break;
			}*/
			int gpio_level = gpio_get_level(io_num);
            if(gpio_level == 1){
            	// rising edge
            	int64_t currentTime = esp_timer_get_time();
				pointer_to_pwm_data->rise_to_rise_time = (float)(currentTime - pointer_to_pwm_data->last_rise_time);
				pointer_to_pwm_data->last_rise_time = currentTime;
            	
            }else if (gpio_level == 0){
            	// falling edge
            	int64_t currentTime = esp_timer_get_time();
            	if(pointer_to_pwm_data->rise_to_rise_time != 0){
	            	pointer_to_pwm_data->duty = 20*( (float)(currentTime - pointer_to_pwm_data->last_rise_time) ) / pointer_to_pwm_data->rise_to_rise_time -1;
	            	if(pointer_to_pwm_data->duty < 0) pointer_to_pwm_data->duty = 0;
	            	if(pointer_to_pwm_data->duty > 1) pointer_to_pwm_data->duty = 1;
	            }
            }
            printf("GPIO[%d] intr, val: %d, duty = %f, rr_time = %f, lr_time = %lld\n", io_num, gpio_level, pointer_to_pwm_data->duty, pointer_to_pwm_data->rise_to_rise_time, pointer_to_pwm_data->last_rise_time);
            
        }
    }
}

void initPWM_Input()
{
    gpio_config_t io_conf;

    //interrupt of rising edge
    io_conf.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins, use GPIO4/5 here
    io_conf.pin_bit_mask = GPIO_INPUT_PIN_SEL;
    //set as input mode    
    io_conf.mode = GPIO_MODE_INPUT;
    //enable pull-up mode
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);

    //change gpio intrrupt type for one pin
    //gpio_set_intr_type(RUDDER_CHANNEL, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(ELEVATOR_CHANNEL, GPIO_INTR_ANYEDGE);
    //gpio_set_intr_type(THRUST_CHANNEL, GPIO_INTR_ANYEDGE);
    //gpio_set_intr_type(POTI_CHANNEL, GPIO_INTR_ANYEDGE);
    //gpio_set_intr_type(SWITCH_SWC_CHANNEL, GPIO_INTR_ANYEDGE);

    //create a queue to handle gpio event from isr
    gpio_evt_queue = xQueueCreate(100, sizeof(uint32_t));
    //start gpio task
    xTaskCreate(gpio_task_example, "gpio_task_example", 2048, NULL, 2 | portPRIVILEGE_BIT , NULL);

    //install gpio isr service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
    //hook isr handler for specific gpio pin
    //gpio_isr_handler_add(RUDDER_CHANNEL, gpio_isr_handler, (void*) RUDDER_CHANNEL);
    gpio_isr_handler_add(ELEVATOR_CHANNEL, gpio_isr_handler, (void*) ELEVATOR_CHANNEL);
    //gpio_isr_handler_add(THRUST_CHANNEL, gpio_isr_handler, (void*) THRUST_CHANNEL);
    //gpio_isr_handler_add(POTI_CHANNEL, gpio_isr_handler, (void*) POTI_CHANNEL);
    //gpio_isr_handler_add(SWITCH_SWC_CHANNEL, gpio_isr_handler, (void*) SWITCH_SWC_CHANNEL);
	//TODO: just to show that it's possible?
	
    //remove isr handler for gpio number.
    //gpio_isr_handler_remove(ELEVATOR_CHANNEL);
    //hook isr handler for specific gpio pin again
    //gpio_isr_handler_add(ELEVATOR_CHANNEL, gpio_isr_handler, (void*) ELEVATOR_CHANNEL);
	

	//TODO: this looks useful:
	//vTaskDelay(1000 / portTICK_RATE_MS);
}

