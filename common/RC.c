#include "../../common/num_config_vars.h"

#define RC_MODE 0
#define DATA_MODE 1
#define LINE_TENSION_REQUEST_MODE 2
#define LINE_LENGTH_MODE 3
#define CONFIG_MODE 4
#define DEBUG_DATA_MODE 5

#define DATALENGTH 4


static uint8_t broadcast_mac[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
const uint8_t WIFI_CHANNEL = 0;

float line_length_in_meters = 1;
float line_speed = 0;
float flight_mode = 0;
float groundstation_height = 0;

int newWiFidata = false;

float tension_request = 0;

static void (*write_config_callback_kite)(float*);

static void (*receive_config_callback_groundstation)(float*);
static void (*receive_debugging_data_callback_groundstation)(float*, int);

typedef struct __attribute__((packed)) esp_now_msg_t
{
	uint32_t mode;
	float data[DATALENGTH];
	// Can put lots of things here
} esp_now_msg_t;

typedef struct __attribute__((packed)) esp_now_msg_t_large
{
	uint32_t mode;
	float data[NUM_CONFIG_FLOAT_VARS];
} esp_now_msg_t_large;

typedef struct __attribute__((packed)) esp_now_msg_t_medium
{
	uint32_t mode;
	float data[6];
} esp_now_msg_t_medium;

typedef struct __attribute__((packed)) esp_now_msg_t_10
{
	uint32_t mode;
	float data[10];
} esp_now_msg_t_10;

typedef struct __attribute__((packed)) esp_now_msg_t_25
{
	uint32_t mode;
	float data[25];
} esp_now_msg_t_25;

int rec_led_state = 0;
// gets called when incoming data is received
static void msg_recv_cb_kite(const uint8_t *mac_addr, const uint8_t *data, int len)
{
	if (len == sizeof(esp_now_msg_t))
	{
		esp_now_msg_t msg;
		memcpy(&msg, data, len);
		
		if(msg.mode == LINE_LENGTH_MODE){
			
			if(rec_led_state == 0){
				rec_led_state = 1;
			}else{
				rec_led_state = 0;
			}
			
			line_length_in_meters = msg.data[0];
			flight_mode = msg.data[1];
			groundstation_height = msg.data[2];
			line_speed = msg.data[3];
			newWiFidata = true;
		}
	}
	
	if (len == sizeof(esp_now_msg_t_large))
	{
		esp_now_msg_t_large msg;
		memcpy(&msg, data, len);
		if(msg.mode == CONFIG_MODE){
			(*write_config_callback_kite)(msg.data);
		}
	}
	
}

//TODO: I think this is not needed anymore, as the groundstation does not receive data from the kite
static void msg_recv_cb_groundstation(const uint8_t *mac_addr, const uint8_t *data, int len)
{
	//printf("received esp-now message of size %d\n", len);
	if (len == sizeof(esp_now_msg_t))
	{
		esp_now_msg_t msg;
		memcpy(&msg, data, len);
		if(msg.mode == LINE_TENSION_REQUEST_MODE){
			tension_request = 1.0; // kite is in landing mode
		}
	}
	
	if (len == sizeof(esp_now_msg_t_large))
	{
		esp_now_msg_t_large msg;
		memcpy(&msg, data, len);
		if(msg.mode == CONFIG_MODE){
			(*receive_config_callback_groundstation)(msg.data);
		}
	}
	
	if (len == sizeof(esp_now_msg_t_medium)){
		esp_now_msg_t_medium msg;
		memcpy(&msg, data, len);
		(*receive_debugging_data_callback_groundstation)(msg.data, 6);
		
	}
	
	if (len == sizeof(esp_now_msg_t_10)){
		esp_now_msg_t_10 msg;
		memcpy(&msg, data, len);
		(*receive_debugging_data_callback_groundstation)(msg.data, 10);
		
	}
	if (len == sizeof(esp_now_msg_t_25)){
		esp_now_msg_t_25 msg;
		memcpy(&msg, data, len);
		(*receive_debugging_data_callback_groundstation)(msg.data, 25);
		
	}
}

void network_setup_common(){
	//TODO: check if neccessary when WIFI_STORAGE_RAM is used
	// Initialize FS NVS
	esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK( nvs_flash_erase() );
		ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	
	
	//ESP_ERROR_CHECK(esp_netif_init());
	
	// Wifi
	wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
	esp_wifi_init(&cfg);
	
	
    esp_wifi_set_storage(WIFI_STORAGE_RAM);
	// Puts ESP in STATION MODE
	
	esp_wifi_set_mode(WIFI_MODE_STA); // MUST BE CALLED AFTER esp_wifi_init(&cfg) to have a non-volatile effect on the flash nvs!
	
	esp_wifi_config_80211_tx_rate(ESP_IF_WIFI_STA, WIFI_PHY_RATE_LORA_250K);// TODO: does this influence the range?
	
	esp_wifi_start();
	
	ESP_ERROR_CHECK( esp_wifi_set_protocol(ESP_IF_WIFI_STA, WIFI_PROTOCOL_LR) );
	
	// ESP NOW
	esp_now_init();
	
	esp_now_peer_info_t peer_info;
	peer_info.channel = WIFI_CHANNEL;
	memcpy(peer_info.peer_addr, broadcast_mac, 6);
	peer_info.ifidx = ESP_IF_WIFI_STA;
	peer_info.encrypt = false;
	esp_now_add_peer(&peer_info);
}

// init wifi on the esp
// register callbacks
void network_setup_kite_flying(void (*write_config_callback_arg)(float*))
{
    network_setup_common();
    
	write_config_callback_kite = write_config_callback_arg;
	
	// Register Send Callback
	esp_now_register_recv_cb(msg_recv_cb_kite);
}

// init wifi on the esp
// register callbacks
void network_setup_groundstation(void (*received_config_callback_arg)(float*), void (*receive_debugging_data_callback_arg)(float*, int))
{
	network_setup_common();
	
	receive_config_callback_groundstation = received_config_callback_arg;
	receive_debugging_data_callback_groundstation = receive_debugging_data_callback_arg;
	
	// Register Receive Callback
	esp_now_register_recv_cb(msg_recv_cb_groundstation); //TODO: I believe this is not needed anymore.
}

void sendDebuggingData(float num1, float num2, float num3, float num4, float num5, float num6){
	esp_now_msg_t_medium msg;
	msg.mode = DEBUG_DATA_MODE;
	msg.data[0] = num1;
	msg.data[1] = num2;
	msg.data[2] = num3;
	msg.data[3] = num4;
	msg.data[4] = num5;
	msg.data[5] = num6;
	
	uint16_t packet_size = sizeof(esp_now_msg_t_medium);
	uint8_t msg_data[packet_size];
	memcpy(&msg_data[0], &msg, sizeof(esp_now_msg_t_medium));
	
	esp_now_send(broadcast_mac, msg_data, packet_size);
}

void sendDebuggingData10(float num1, float num2, float num3, float num4, float num5, float num6, float num7, float num8, float num9, float num10){
	esp_now_msg_t_10 msg;
	msg.mode = DEBUG_DATA_MODE;
	msg.data[0] = num1;
	msg.data[1] = num2;
	msg.data[2] = num3;
	msg.data[3] = num4;
	msg.data[4] = num5;
	msg.data[5] = num6;
	msg.data[6] = num7;
	msg.data[7] = num8;
	msg.data[8] = num9;
	msg.data[9] = num10;
	
	uint16_t packet_size = sizeof(esp_now_msg_t_10);
	uint8_t msg_data[packet_size];
	memcpy(&msg_data[0], &msg, sizeof(esp_now_msg_t_10));
	
	esp_now_send(broadcast_mac, msg_data, packet_size);
}

void sendDebuggingData25(float num1, float num2, float num3, float num4, float num5, float num6, float num7, float num8, float num9, float num10, float num11, float num12, float num13, float num14, float num15, float num16, float num17, float num18, float num19, float num20, float num21, float num22, float num23, float num24, float num25){
	esp_now_msg_t_25 msg;
	msg.mode = DEBUG_DATA_MODE;
	msg.data[0] = num1;
	msg.data[1] = num2;
	msg.data[2] = num3;
	msg.data[3] = num4;
	msg.data[4] = num5;
	msg.data[5] = num6;
	msg.data[6] = num7;
	msg.data[7] = num8;
	msg.data[8] = num9;
	msg.data[9] = num10;
	msg.data[10] = num11;
	msg.data[11] = num12;
	msg.data[12] = num13;
	msg.data[13] = num14;
	msg.data[14] = num15;
	msg.data[15] = num16;
	msg.data[16] = num17;
	msg.data[17] = num18;
	msg.data[18] = num19;
	msg.data[19] = num20;
	msg.data[20] = num21;
	msg.data[21] = num22;
	msg.data[22] = num23;
	msg.data[23] = num24;
	msg.data[24] = num25;
	
	uint16_t packet_size = sizeof(esp_now_msg_t_25);
	uint8_t msg_data[packet_size];
	memcpy(&msg_data[0], &msg, sizeof(esp_now_msg_t_25));
	
	esp_now_send(broadcast_mac, msg_data, packet_size);
}

/*
// used by the kite to send data to the data receiver
void sendDataArray(float data[DATALENGTH], uint32_t mode){

	esp_now_msg_t msg;
	
	msg.mode = mode;
	for(int i = 0; i < DATALENGTH; i++){
		msg.data[i] = data[i];
	}
	
	// Pack
	uint16_t packet_size = sizeof(esp_now_msg_t);
	uint8_t msg_data[packet_size]; // Byte array
	memcpy(&msg_data[0], &msg, sizeof(esp_now_msg_t));
	
	// Send
	esp_now_send(broadcast_mac, msg_data, packet_size);
}
*/

void sendData(uint32_t mode, float data0, float data1, float data2, float data3){
	
	esp_now_msg_t msg;
	
	msg.mode = mode;
	msg.data[0] = data0;
	msg.data[1] = data1;
	msg.data[2] = data2;
	msg.data[3] = data3;
	
	// Pack
	uint16_t packet_size = sizeof(esp_now_msg_t);
	uint8_t msg_data[packet_size]; // Byte array
	memcpy(&msg_data[0], &msg, sizeof(esp_now_msg_t));
	
	// Send
	esp_now_send(broadcast_mac, msg_data, packet_size);
}

void sendDataArrayLarge(uint32_t mode, float* data, int length){
	esp_now_msg_t_large msg;
	msg.mode = mode;
	for(int i = 0; i < length; i++){
		msg.data[i] = data[i];
	}
	// Pack
	uint16_t packet_size = sizeof(esp_now_msg_t_large);
	uint8_t msg_data[packet_size]; // Byte array
	memcpy(&msg_data[0], &msg, sizeof(esp_now_msg_t_large));
	
	// Send
	esp_now_send(broadcast_mac, msg_data, packet_size);
}

void processRC(){
	// placeholder for thread-free proxy version RC_proxy.c
}
