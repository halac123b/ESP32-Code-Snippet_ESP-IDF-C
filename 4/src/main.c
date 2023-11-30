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

#include "mqtt_client.h" //provides important functions to connect with MQTT

// Wifi name and password
const char *ssid = "Honda";
const char *pass = "honda123";

int retry_num = 0;

const char *TAG = "MQTT_TCP";

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

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data){
  // Making obj client of struct esp_mqtt_client_handle_t and assigning it the receieved event client
  esp_mqtt_event_handle_t event = event_data;
  esp_mqtt_client_handle_t client = event->client;

  if(event->event_id == MQTT_EVENT_CONNECTED){
    ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
    // In mqtt we require a topic to subscribe and client is from event client and 0 is quality of service it can be 1 or 2
    esp_mqtt_client_subscribe(client, "your topic", 0);
    ESP_LOGI(TAG, "sent subscribe successful");
    printf("sent subscribe successful");
  }
  else if(event->event_id == MQTT_EVENT_DISCONNECTED){
    ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED"); //if disconnected
    printf("MQTT_EVENT_DISCONNECTED");
  }
  else if(event->event_id == MQTT_EVENT_SUBSCRIBED){  //when subscribed
    ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED");
    printf("MQTT_EVENT_SUBSCRIBED");
  }
  else if(event->event_id == MQTT_EVENT_UNSUBSCRIBED){ //when unsubscribed
    ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED");
    printf("MQTT_EVENT_UNSUBSCRIBED");
  }
  else if(event->event_id == MQTT_EVENT_DATA){  // Khi có data được gửi đến
    ESP_LOGI(TAG, "MQTT_EVENT_DATA");
    printf("Received topic=%.*s, data=%.*s\n", event->topic_len, event->topic, event->data_len, event->data);
  }
  else if(event->event_id == MQTT_EVENT_ERROR){ //when any error
    ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
    printf("MQTT_EVENT_ERROR");
  }
}

// Init MQTT protocol
static void mqtt_app_start(){
  // Depending on your website or cloud there could be more parameters in mqtt_cfg
  const esp_mqtt_client_config_t mqtt_cfg = {
    .broker = {
      .address = {
        .uri = "mqtt://io.adafruit.com", // Uniform Resource Identifier includes path, protocol
        .port = 1883
      }
    },
    .credentials = {
      .username = "halac123b",  // your Adafruit username
      .authentication = {
        .password = "aio_tFpj54bK3UXb0kLL4ggXe7P2q8wB" // your Adafruit password
      }
    }
  };

  esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg); //sending struct as a parameter in init client function
  esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL); // Register with event handler
  esp_mqtt_client_start(client); //starting the process


  // Subcribe to a specific feed, when receive data, the event MQTT_EVENT_DATA will trigger
  // Only need to call this function once, after subcribe success, trigger event MQTT_EVENT_SUBSCRIBED
  esp_mqtt_client_subscribe(client, "halac123b/feeds/Humid", 0);
  while (1) {
      // Publish to a specific feed
      esp_mqtt_client_publish(client, "halac123b/feeds/Humid", "50", 0, 1, 0);
      vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}

void app_main(void){
  nvs_flash_init();
  wifi_connection();

  vTaskDelay(10000 / portTICK_PERIOD_MS); //delay is important cause we need to let it connect to wifi

  mqtt_app_start(); // MQTT start app as shown above most important code for MQTT
}