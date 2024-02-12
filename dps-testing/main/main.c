#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#include "../../common/i2c_devices/dps310.h"


#include "nvs_flash.h"


struct i2c_bus bus0 = {18, 19};
struct i2c_bus bus1 = {25, 14};

void main_task(void* arg)
{
	
	init_dps310(bus1);
	
	while(1) {
		vTaskDelay(1);
		update_dps310_if_necessary();
		printf("height is %f.\n", getHeight());
		
	}
}

void app_main(void){
	xTaskCreate(main_task, "main_task", 20000, NULL, 17, NULL);
}
