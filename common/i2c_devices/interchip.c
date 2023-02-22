#include "freertos/FreeRTOS.h"
#include "driver/i2c.h"
#include "interchip.h"
#include "esp_err.h"

#define I2X_FREQUENCY 100000
#define I2C_PORT_T 0

// ToDo 2 instances of driver, one per bus?
esp_err_t _initInterchip(struct i2c_bus bus) {
	//this line should be after i2c_param_config according to espressif documentation
    i2c_driver_install(I2C_PORT_T, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = bus.sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = bus.scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2X_FREQUENCY
    };
    return i2c_param_config(I2C_PORT_T, &conf);
}


int i2c_receive(struct i2c_bus bus, int chip_addr, int data_addr, int len) {
    uint8_t *data = malloc(len);
    _initInterchip(bus);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    if (data_addr != -1) {
        i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
        i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
        i2c_master_start(cmd);
    }
    i2c_master_write_byte(cmd, chip_addr << 1 | READ_BIT, ACK_CHECK_EN);
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, ACK_VAL);
    }
    i2c_master_read_byte(cmd, data + len - 1, NACK_VAL);
    i2c_master_stop(cmd);
    /*esp_err_t ret = */i2c_master_cmd_begin(I2C_PORT_T, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    int returnValue = data[0];
    free(data);
    i2c_driver_delete(I2C_PORT_T);
    return returnValue;
}



int i2c_send(struct i2c_bus bus, int chip_addr, int data_addr, int data, int len) {

	_initInterchip(bus);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
    
    for (int i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(I2C_PORT_T, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
        //printf("i2c: Write OK");
    } else if (ret == ESP_ERR_TIMEOUT) {
        printf("i2c: Bus is busy");
    } else {
        printf("i2c: Write Failed");
    }
    i2c_driver_delete(I2C_PORT_T);
    return 0;
}
