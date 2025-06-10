/* WIFI-UART translation
*/

#include <math.h>
#include <string.h>

#include <sys/unistd.h>
#include <sys/stat.h>

#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"

#include "timer.c"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_now.h"

#include "driver/uart.h"

#include "driver/gpio.h"

#include "driver/mcpwm.h"

#include "mymath.c"


#include "my_gpio.c"

#include "../../common/RC.c"

#include "driver/ledc.h"


#include "esp_system.h"
#include "driver/spi_master.h"


float data_including_lora[10];

void processReceivedConfigValuesViaWiFi(float* config_values){}

FILE *f;
int file_open = false;

int new_data_available = false;

float debugging_data[25];

void processReceivedDebuggingDataViaWiFi(float* debugging_data_arg, int length){
	for(int i = 0; i < length; i++){
		debugging_data[i] = debugging_data_arg[i];
	}
	new_data_available = true;
}

void init(){
	
	initGPIO();
	
	network_setup_groundstation(&processReceivedConfigValuesViaWiFi, &processReceivedDebuggingDataViaWiFi);
	
	init_uptime();
	
	for(int i = 0; i < 10; i++){
		data_including_lora[i] = 0;
	}
	for(int i = 0; i < 25; i++){
		debugging_data[i] = 0;
	}
}

static const char *TAG = "example";

void app_main(void){
	init();
	
	esp_log_level_set("sdmmc_sd", ESP_LOG_VERBOSE);
	esp_log_level_set("sdmmc_cmd", ESP_LOG_VERBOSE);

	esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};
	sdmmc_card_t *card;
	const char mount_point[] = "/sdcard";
	
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	//host.max_freq_khz = 200;
	
	spi_bus_config_t bus_cfg = {
		.mosi_io_num = 13,
		.miso_io_num = 26,
		.sclk_io_num = 15,
		.quadwp_io_num = -1,
		.quadhd_io_num = -1,
		.max_transfer_sz = 4000,
	};
	
	esp_err_t ret;
	ret = spi_bus_initialize(host.slot, &bus_cfg, SDSPI_DEFAULT_DMA);
	if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }
	
	
	
	// This initializes the slot without card detect (CD) and write protect (WP) signals.
	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
	slot_config.gpio_cs = 5;
	slot_config.host_id = host.slot;
	
	gpio_set_pull_mode(5, GPIO_PULLUP_ONLY);  // Chip Select pin
	gpio_set_pull_mode(26, GPIO_PULLUP_ONLY); // MISO pin
	
	ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);
	
	if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");
	
	
	
	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	const char *file_hello = "/sdcard/diagram";
	const char *ext = ".kd3";
	
	int i = 0;
	int free_filename_found = false;
	struct stat st;
	char dest[128];
	while(!free_filename_found){
		snprintf(dest, 128, "%s%d%s", file_hello, i, ext);
		i++;
		if (stat(dest, &st) == 0) {
			//go to next file
		}else{
			free_filename_found = true;
		}
	}
	
	
	f = fopen(dest, "w");
	file_open = true;
	fprintf(f, "[[0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0]");
	

	//FILE *file = fopen("/sdcard/data.bin", "wb");
	//fwrite(data, sizeof(float), num_elements, f);
	//fprintf(f, "%.5g", my_float)

	//fclose(f);
	
	while(1){
		
		// **************** FILE CLOSE SWITCH ****************
		if(file_open && get_level_GPIO_22() == 0){
			// close logging file
			fprintf(f, "]");
			printf("closing file\n");
			fclose(f);
			file_open = false;
		}
		
		if(file_open && new_data_available){
			
			new_data_available = false;
			
			fprintf(f, ",[%f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f, %f]", debugging_data[0], debugging_data[1], debugging_data[2], debugging_data[3], debugging_data[4], debugging_data[5], debugging_data[6], debugging_data[7], debugging_data[8], debugging_data[9], debugging_data[10], debugging_data[11], debugging_data[12], debugging_data[13], debugging_data[14], debugging_data[15], debugging_data[16], debugging_data[17], debugging_data[18], debugging_data[19], debugging_data[20], debugging_data[21], debugging_data[22], debugging_data[23], debugging_data[24], get_uptime_seconds());
			
			printf("writing to sd\n");
		}
		
	    vTaskDelay(2.0);
    }
    
}
