#define RX_BUF_SIZE (1024)

//tx_pin/rx_pin = GPIO_NUM_13 ...
//uart_num = UART_NUM_1 or UART_NUM_2
void initUART(uart_port_t uart_num, gpio_num_t tx_pin, gpio_num_t rx_pin)
{
	uart_config_t uart_config = {
    	.baud_rate = 115200,
    	.data_bits = UART_DATA_8_BITS,
    	.parity = UART_PARITY_DISABLE,
    	.stop_bits = UART_STOP_BITS_1,
    	.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    	.source_clk = UART_SCLK_APB,
	};
	uart_driver_install(uart_num, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(uart_num, &uart_config);
    uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
}

void sendUARTArray100(float* array, int length, uart_port_t uart_num){
	float d[102];
	d[0] = 314;
	for(int i = 0; i < length; i++){
		d[i+1] = array[i];
	}
	d[length+1] = 314;
	
	uint8_t to_be_sent[102*4];
	memcpy(to_be_sent, d, 102*4);
	uart_write_bytes(uart_num, &to_be_sent, 102*4);
}

void sendUART(float number1, float number2, uart_port_t uart_num){
	float d[4];
    d[0] = 314;
    d[1] = number1;
    d[2] = number2;
    d[3] = 314;
    
    uint8_t d_b[16];
    memcpy(d_b, d, 16);
    uint8_t to_be_sent[16];
    for(int i = 0; i < 4; i++){
    	for(int j = 0; j < 4; j++){
			memcpy(to_be_sent+j + 4*i, d_b+3-j + 4*i, 1);
		}
	}
    
    uart_write_bytes(uart_num, &to_be_sent, 16);
}

static uint8_t data[RX_BUF_SIZE+1];


int receiveUARTArray100(float* array, int* length, uart_port_t uart_num){
	int received_byte_length = 0;
	uart_get_buffered_data_len(uart_num, (size_t*)&received_byte_length);
	printf("Length is %d\n", received_byte_length);
	int num_floats = (received_byte_length)/4;
	if(num_floats > 102) num_floats = 102;
	const int rxBytes = uart_read_bytes(uart_num, data, RX_BUF_SIZE+1, 0);
	if(rxBytes == 0) return 0;
	float total_array[102];
	memcpy(total_array, data, num_floats*4);
	if(total_array[0] != 314){
		//try reversing byte order of floats
		uint8_t reversed_data[4*num_floats];
       	//memcpy(e_b, data, 16);
       	for(int i = 0; i < num_floats; i++){
       		for(int j = 0; j < 4; j++){
	        	memcpy(reversed_data+j + 4*i, data+3-j + 4*i, 1);
	       	}
    	}
	   	memcpy(total_array, reversed_data, num_floats*4);
	}
	if(total_array[0] != 314) return 0;
	int i = 1;
	int end_found = 0;
	while(end_found == 0 && i < num_floats){
		if(total_array[i] == 314){
			end_found = 1;
			i++;
		}else{
			i++;
		}
	}
	if(end_found == 0) return 0;
	*length = i-2;
	memcpy(array, total_array+1, (*length)*4);
	return 1;
}

