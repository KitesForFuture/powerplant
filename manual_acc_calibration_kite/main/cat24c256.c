static float i2c_receive_16bitDataAddress(uint8_t bus_number, int chip_addr, int data_addr)
{
	esp_err_t ret = ESP_FAIL;
	int no_of_try = 0;
	
	uint8_t *data = malloc(4);
	do{
		_initInterchip(bus_number);
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd); // START
		data_addr*=4;
		//if (data_addr != -1) {
		    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN); // CHIP ADDR + WRITE BIT
		    
		    // DATA ADDR
		    i2c_master_write_byte(cmd, data_addr>>8, ACK_CHECK_EN);// right shifts by 8 bits, thus cutting off the 8 smallest bits
			i2c_master_write_byte(cmd, data_addr&255, ACK_CHECK_EN);//bitwise AND removes all but the last 8 bits
		    
		    i2c_master_start(cmd); // START
		//}
		i2c_master_write_byte(cmd, chip_addr << 1 | READ_BIT, ACK_CHECK_EN); // CHIP ADDR + READ BIT
		
		//i2c_master_read(cmd, data, 4, NACK_VAL); // READ + NO ACK
		i2c_master_read(cmd, data, 3, ACK_VAL);
		i2c_master_read_byte(cmd, data + 3, NACK_VAL);
		
		i2c_master_stop(cmd); // STOP
		ret = i2c_master_cmd_begin(i2c_port, cmd, 1000 / portTICK_RATE_MS);
		i2c_cmd_link_delete(cmd);
		i2c_driver_delete(i2c_port);
		no_of_try++;
		//printf("tried, ret = %d\n", ret);
	}while(ret != ESP_OK && no_of_try < 10);
    //printf("ret = %d. ESP_FAIL = %d, ESP_OK = %d\n", ret, ESP_FAIL, ESP_OK);
    
    union Conversion {
		int i;
		float f;
	} conversion;
	conversion.i = ((int)data[0]) + ((int)data[1])*256 + ((int)data[2])*256*256 + ((int)data[3])*256*256*256;
    //printf("data = %d, %f\n", conversion.i, conversion.f);
    
    free(data);
    
    return conversion.f;
}



static int i2c_send_16bitDataAddress(uint8_t bus_number, int chip_addr, int data_addr, float data, int len)
{
	union Conversion {
		int i;
		float f;
	} conversion;  
	conversion.f = data;
	
	_initInterchip(bus_number);
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd); // START
    i2c_master_write_byte(cmd, chip_addr << 1 | WRITE_BIT, ACK_CHECK_EN);
    data_addr*=4;
    i2c_master_write_byte(cmd, data_addr>>8, ACK_CHECK_EN);// right shifts by 8 bits, thus cutting off the 8 smallest bits
    i2c_master_write_byte(cmd, data_addr&255, ACK_CHECK_EN);//bitwise AND removes all but the last 8 bits
    for (int i = 0; i < len; i++) {
        i2c_master_write_byte(cmd, ((conversion.i)>>(8*i))&255, ACK_CHECK_EN);
    }
    // above loop could be replaced by something like
    // i2c_master_write(cmd, &(conversion.i), ACK_CHECK_EN);
    i2c_master_stop(cmd); // STOP
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


void write2EEPROM(float number, int address){
	//i2c_send(bus_no(1 for bmp and cat), chip_address, data_addr, data, length);
	i2c_send_16bitDataAddress(1, 0x50, address, number, 4);
}

float readEEPROM(int address){
	//i2c_send(bus_no(1 for bmp and cat), chip_address, data_addr, data, length);
	i2c_receive_16bitDataAddress(1, 0x50, address); // THIS IS A DIRTY HACK, because when data_addr != 0 and a write has just taken place, a read returns garbage. TODO: Why?
	return i2c_receive_16bitDataAddress(1, 0x50, address);
}
