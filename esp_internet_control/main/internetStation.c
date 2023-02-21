/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "esp_tls.h"
#include "esp_crt_bundle.h"

#define EXAMPLE_ESP_WIFI_SSID      "PYUR C7E0C"
#define EXAMPLE_ESP_WIFI_PASS      "kKf8333jsCZk"
#define EXAMPLE_ESP_MAXIMUM_RETRY  10

#define WEB_URL "https://kitesforfuture.de"
#define REQUEST_FORMAT "GET /control/getCommand.php?name=%s&status=%s&line_length=%d HTTP/1.0\r\nHost: kitesforfuture.de:80\r\n\r\n"
#define KITE_NAME "kite1"

#define LAUNCH_COMMAND 0
#define LAND_COMMAND 1

#define LAUNCHING_STATUS 0
#define EIGHT_STATUS 1
#define RETRACTING_STATUS 2
#define LANDING_STATUS 3

float current_command = LAND_COMMAND;
float current_status = LAUNCHING_STATUS;
float current_line_length = 0;


//************************************************************
//******************** WIFI STATION **************************
//************************************************************


/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static const char *TAG = "wifi station, http";

static int s_retry_num = 0;

static void event_handler(void* arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        ESP_LOGI(TAG,"connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .password = EXAMPLE_ESP_WIFI_PASS,
            /* Setting a password implies station will connect to all security modes including WEP/WPA.
             * However these modes are deprecated and not advisable to be used. Incase your Access point
             * doesn't support WPA2, these mode can be enabled by commenting below line */
	     .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    ESP_LOGI(TAG, "wifi_init_sta finished.");

    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS);
    } else {
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}


//************************************************************
//********************* HTTPS REQUEST ************************
//************************************************************


static char request_string[256];
static const char string_landing[] = "landing";
static const char string_launching[] = "launching";
static const char string_eight[] = "eight";
static const char string_retracting[] = "retracting";

static void https_get_request_using_crt_bundle(char* responseArg, int* lenArg)
{
    //ESP_LOGI(TAG, "https_request using crt bundle");
    esp_tls_cfg_t cfg = {
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    
    char buf[512];
    int ret, len;

    struct esp_tls *tls = esp_tls_conn_http_new(WEB_URL, &cfg);

    if (tls != NULL) {
        //ESP_LOGI(TAG, "Connection established...");
    } else {
        ESP_LOGE(TAG, "Connection failed...");
        goto exit;
    }
	
	int request_string_length = 0;
	if(current_status == LANDING_STATUS){
		request_string_length = sprintf(request_string, REQUEST_FORMAT, KITE_NAME, string_landing, (int)current_line_length);
	}else if(current_status == LAUNCHING_STATUS){
		request_string_length = sprintf(request_string, REQUEST_FORMAT, KITE_NAME, string_launching, (int)current_line_length);
	}else if(current_status == EIGHT_STATUS){
		request_string_length = sprintf(request_string, REQUEST_FORMAT, KITE_NAME, string_eight, (int)current_line_length);
	}else if(current_status == RETRACTING_STATUS){
		request_string_length = sprintf(request_string, REQUEST_FORMAT, KITE_NAME, string_retracting, (int)current_line_length);
	}
	
    size_t written_bytes = 0;
    do {
        ret = esp_tls_conn_write(tls,
                                 request_string + written_bytes,
                                 request_string_length - written_bytes);
        if (ret >= 0) {
            //ESP_LOGI(TAG, "%d bytes written", ret);
            written_bytes += ret;
        } else if (ret != ESP_TLS_ERR_SSL_WANT_READ  && ret != ESP_TLS_ERR_SSL_WANT_WRITE) {
            ESP_LOGE(TAG, "esp_tls_conn_write  returned: [0x%02X](%s)", ret, esp_err_to_name(ret));
            goto exit;
        }
    } while (written_bytes < request_string_length);

    //ESP_LOGI(TAG, "Reading HTTP response...");
	
	int j = 0;
	*lenArg = 0;
	
    do {
        len = sizeof(buf) - 1;
        bzero(buf, sizeof(buf));
        ret = esp_tls_conn_read(tls, (char *)buf, len);

        if (ret == ESP_TLS_ERR_SSL_WANT_WRITE  || ret == ESP_TLS_ERR_SSL_WANT_READ) {
            continue;
        }

        if (ret < 0) {
            ESP_LOGE(TAG, "esp_tls_conn_read  returned [-0x%02X](%s)", -ret, esp_err_to_name(ret));
            break;
        }

        if (ret == 0) {
            //ESP_LOGI(TAG, "connection closed");
            break;
        }
        len = ret;
        
		ESP_LOGD(TAG, "%d bytes read", len);
        //Print response directly to stdout as it is read
        for (int i = 0; i < len; i++) {
            responseArg[j] = buf[i];
            j++;
        }
        *lenArg += len;
        responseArg[j] ='\n'; // JSON output doesn't have a newline at end
        
    } while (1);

exit:
    esp_tls_conn_delete(tls);
}

static void https_request_task(void *pvparameters)
{
    ESP_LOGI(TAG, "Start https_request example");
	
	char response[1000];
	int len = 0;
	
	while(1){
	    https_get_request_using_crt_bundle(response, &len);
	    
        ESP_LOGD(TAG, "%d bytes read", len);
        
        if (response[len-2] == 'D'){
        	current_command = LAND_COMMAND;
        	//current_status = LANDING_STATUS;
        }else if (response[len-2] == 'H'){
        	current_command = LAUNCH_COMMAND;
        	//current_status = LAUNCHING_STATUS;
        }
        
	    //ESP_LOGI(TAG, "Finish https_request example");
	    vTaskDelay(9);
	}
    
    vTaskDelete(NULL);
}

void init_internet_connection(void)
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_STA");
    wifi_init_sta();
    xTaskCreate(&https_request_task, "https_get_task", 8192, NULL, 5, NULL);
}
