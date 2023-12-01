#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"

static const char *TAG = "uart_events";

// 2 pin of UART on esp32 wemos d1 r32
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
// 2 pin trên tương ứng module UART thứ 2 của board
#define UART_MODULE (UART_NUM_2)

#define PATTERN_CHR_NUM    (10)         /* Chiều dài của pattern nhận từ UART */

#define BUF_SIZE (1024)

#define PATTERN '!'

// Queue chứa Uart event
static QueueHandle_t uart_queue;

// Get data value from pattern
char* extract_pattern(char *input){
  char temp[10];
  size_t i;
  for (i = 0; i < strlen(input) && input[i] != '#'; i++){
    temp[i] = input[i];
  }
  temp[i] = '\0'; // End substring

  char temp_str[5];
  sscanf(temp, "!temp:%4[^#]#", temp_str);
  return temp_str;
}

static void uart_event_task(void *pvParameters){
  // Structure biểu diễn UART event
  uart_event_t event;
  size_t buffered_size;

  // Tạo biến để nhận data
  uint8_t *data = (uint8_t*) malloc(BUF_SIZE);
  while (1){
    /* Waiting for UART event
      uart_queue: QueueHandle_t, queue chứa các event được nhận
      event: uart_event_t, event nhận từ queue copy vào đây
      timeout: thời gian lệnh này sẽ block task để đợi event đến
    */
    if (xQueueReceive(uart_queue, (void * )&event, 5000 / portTICK_PERIOD_MS)){
      // Set giá trị của khối memory data về 0 trước khi write
      bzero(data, BUF_SIZE);
      ESP_LOGI(TAG, "uart[%d] event:", UART_MODULE);

      switch(event.type) {
        // Event of UART receving data
        /* We'd better handler data event fast, there would be much more data events than
          other types of events. If we take too much time on data event, the queue might
          be full.*/
        case UART_DATA:
          ESP_LOGI(TAG, "[UART DATA]: %d", event.size);
          // Read from Uart
          uart_read_bytes(UART_MODULE, data, event.size, 500 / portTICK_PERIOD_MS);

          // Gửi lại data vừa nhận được lại uart
          // uart_write_bytes(UART_MODULE, (const char*) data, event.size);
          break;

        //Event of HW FIFO overflow detected
        case UART_FIFO_OVF:
          ESP_LOGI(TAG, "hw fifo overflow");
          // If fifo overflow happened, you should consider adding flow control for your application.
          // The ISR has already reset the rx FIFO,
          // As an example, we directly flush the rx buffer here in order to read more data.
          uart_flush_input(UART_MODULE);
          // Sau đó reset lại event queue
          xQueueReset(uart_queue);
          break;

        //Event of UART ring buffer full
        case UART_BUFFER_FULL:
          ESP_LOGI(TAG, "ring buffer full");
          // If buffer full happened, you should consider encreasing your buffer size
          // As an example, we directly flush the rx buffer here in order to read more data.
          // Cách xử lí tương tự Overflow
          uart_flush_input(UART_MODULE);
          xQueueReset(uart_queue);
          break;

        //Event of UART RX break detected
        case UART_BREAK:
          ESP_LOGI(TAG, "uart rx break");
          break;

        //Event of UART parity check error
        case UART_PARITY_ERR:
          ESP_LOGI(TAG, "uart parity error");
          break;

        //Event of UART frame error
        case UART_FRAME_ERR:
          ESP_LOGI(TAG, "uart frame error");
          break;

        //UART PATTERN DETECTED
        case UART_PATTERN_DET:
          // Get storage hiện tại của buffer
          uart_get_buffered_data_len(UART_MODULE, &buffered_size);
          // Get vị trí của pattern được tìm thấy (!)
          int pos = uart_pattern_pop_pos(UART_MODULE);
          ESP_LOGI(TAG, "[UART PATTERN DETECTED] pos: %d, buffered size: %d", pos, buffered_size);
          if (pos == -1) {
            // There used to be a UART_PATTERN_DET event, but the pattern position queue is full so that it can not
            // record the position. We should set a larger queue size.
            // As an example, we directly flush the rx buffer here.
            uart_flush_input(UART_MODULE);
          }
          else {
            uart_read_bytes(UART_MODULE, data, pos, 100 / portTICK_PERIOD_MS);
            uint8_t pat[PATTERN_CHR_NUM + 1];
            memset(pat, 0, sizeof(pat));
            uart_read_bytes(UART_MODULE, pat, PATTERN_CHR_NUM, 100 / portTICK_PERIOD_MS);



            char* temperature = extract_pattern((char*)pat);

            // Publish to a specific feed
            esp_mqtt_client_publish(client, "halac123b/feeds/Humid", temperature, 0, 1, 0);

            ESP_LOGI(TAG, "read data: %s", data);
            ESP_LOGI(TAG, "read temperature : %s", temperature);
          }
        break;

        //Others
        default:
            ESP_LOGI(TAG, "uart event type: %d", event.type);
            break;
      }
    }
  }

  free(data);
  data = NULL;
  vTaskDelete(NULL);
}

void initUart(){
  /* Configure parameters of an UART driver,
     * communication pins and install the driver */
  uart_config_t uart_config = {
      .baud_rate = 115200,
      .data_bits = UART_DATA_8_BITS,
      .parity = UART_PARITY_DISABLE,
      .stop_bits = UART_STOP_BITS_1,
      .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
  };

  uart_param_config(UART_MODULE, &uart_config);

  //Set UART pins (using UART0 default pins ie no changes.)
  uart_set_pin(UART_MODULE, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
  //Install UART driver, and get the queue.
    // 20: queue size
  uart_driver_install(UART_MODULE, BUF_SIZE * 2, 0, 20, &uart_queue, 0);

  // Set uart pattern detect function, the pattern is !temp:20#, so we should detect '!'
  uart_enable_pattern_det_intr(UART_MODULE, PATTERN, 1, 10000, 10, 10);
  //Reset the pattern queue length to record at most 20 pattern positions.
  uart_pattern_queue_reset(UART_MODULE, 20);
}

void app_main(){
  initUart();

  vTaskDelay(1000 / portTICK_PERIOD_MS);

  esp_log_level_set(TAG, ESP_LOG_INFO);

  // Create a task to handler UART event from ISR
  xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, configMAX_PRIORITIES, NULL);
}