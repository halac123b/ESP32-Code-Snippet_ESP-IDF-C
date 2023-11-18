#include <stdio.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "esp_system.h"
#include "esp_wifi.h" // Wifi functions of ESP
#include "esp_log.h" // For showing logs
#include "esp_event.h" // Create event with ESP

#include "nvs_flash.h" //non volatile storage
#include "lwip/err.h" //light weight ip packets error handling
#include "lwip/sys.h" //system applications for light weight ip apps

// Wifi name and password
const char *ssid = "C6.13";
const char *pass = "passla123456";

int retry_num = 0;

// Event, đc trigger khi có sự kiện liên quan tới Wifi
static void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id,void *event_data){
  if (event_id == WIFI_EVENT_STA_START){ // Lúc khởi động Wifi và bắt đầu kết nối với Wifi.
    printf("WIFI CONNECTING....\n");
  }
  else if (event_id == WIFI_EVENT_STA_CONNECTED){
    printf("WiFi CONNECTED\n");
  }
  else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
  {
    printf("WiFi lost connection\n");
    // Retry connect lại 5 lần
    if (retry_num < 5){
      esp_wifi_connect(); // Hàm connect ESP với Wifi
      retry_num++;
      printf("Retrying to Connect...\n");
    }
  }
  else if (event_id == IP_EVENT_STA_GOT_IP)
  {
    printf("Wifi got IP...\n\n");
  }
}

void wifi_connection(){
  // 1. Wi-Fi Configuration Phase
  esp_netif_init();
  esp_event_loop_create_default();     // event loop
  esp_netif_create_default_wifi_sta(); // WiFi station

  wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
  esp_wifi_init(&wifi_initiation);

  // Register event handlers
  esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
  esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

  // Wifi config info
  wifi_config_t wifi_configuration = {
    .sta = {
      .ssid = "",
      .password = "",
    }
  };
  strcpy((char*)wifi_configuration.sta.ssid, ssid);
  strcpy((char*)wifi_configuration.sta.password, pass);
  //esp_log_write(ESP_LOG_INFO, "Kconfig", "SSID=%s, PASS=%s", ssid, pass);
  esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);  // Set config info to board

  // 2. Wi-Fi Start Phase
  esp_wifi_start();
  esp_wifi_set_mode(WIFI_MODE_STA);

  // 4. Wi-Fi Connect Phase
  esp_wifi_connect();
  printf("wifi_init_softap finished. SSID:%s  password:%s", ssid, pass);
}

void app_main(void){
  nvs_flash_init();
  wifi_connection();
}