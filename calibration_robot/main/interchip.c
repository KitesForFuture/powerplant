#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define WRITE_BIT I2C_MASTER_WRITE  /*!< I2C master write */
#define READ_BIT I2C_MASTER_READ    /*!< I2C master read */
#define ACK_CHECK_EN 0x1            /*!< I2C master will check ack from slave*/
#define ACK_CHECK_DIS 0x0           /*!< I2C master will not check ack from slave */
#define ACK_VAL 0x0                 /*!< I2C ack value */
#define NACK_VAL 0x1                /*!< I2C nack value */

//I2C frequency
static uint32_t i2c_frequency = 100000;
static i2c_port_t i2c_port = I2C_NUM_0;

static esp_err_t _initInterchip(uint8_t bus_number)
{
	gpio_num_t i2c_gpio_sda = i2c_gpio_sda_bus0;
	gpio_num_t i2c_gpio_scl = i2c_gpio_scl_bus0;
	if(bus_number == 1){
		i2c_gpio_sda = i2c_gpio_sda_bus1;
		i2c_gpio_scl = i2c_gpio_scl_bus1;
	}
	
	
	//this line should be after i2c_param_config according to espressif documentation
    i2c_driver_install(i2c_port, I2C_MODE_MASTER, I2C_MASTER_RX_BUF_DISABLE, I2C_MASTER_TX_BUF_DISABLE, 0);
    
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = i2c_gpio_sda,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = i2c_gpio_scl,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = i2c_frequency
    };
    return i2c_param_config(i2c_port, &conf);
}


static int i2c_receive(uint8_t bus_number, int chip_addr, int data_addr, int len)
{
    uint8_t *data = malloc(len);
    _initInterchip(bus_number);
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
    /*esp_err_t ret = */i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    
    int returnValue = data[0];
    free(data);
    i2c_driver_delete(i2c_port);
    return returnValue;
}



static int i2c_send(uint8_t bus_number, int chip_addr, int data_addr, int data, int len)
{
	_initInterchip(bus_number);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    i2c_master_write_byte(cmd, data_addr, ACK_CHECK_EN);
    for (int i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, data, ACK_CHECK_EN);
    }
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
    i2c_cmd_link_delete(cmd);
    if (ret == ESP_OK) {
        //printf("i2c: Write OK");
    } else if (ret == ESP_ERR_TIMEOUT) {
        printf("i2c: Bus is busy");
    } else {
        printf("i2c: Write Failed");
    }
    i2c_driver_delete(i2c_port);
    return 0;
}
