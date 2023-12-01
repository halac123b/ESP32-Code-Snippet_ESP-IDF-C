#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "esp_log.h"

/**
 * This is an example which echos any data it receives on configured UART back to the sender,
 * with hardware flow control turned off. It does not use UART driver event queue.
 *
 * - Port: configured UART
 * - Receive (Rx) buffer: on
 * - Transmit (Tx) buffer: off
 * - Flow control: off
 * - Event queue: off
 * - Pin assignment: see defines below (See Kconfig)
 */

#define UART_BAUD_RATE     (115200)

// 2 pin of UART on esp32 wemos d1 r32
#define TXD_PIN (GPIO_NUM_17)
#define RXD_PIN (GPIO_NUM_16)
// 2 pin trên tương ứng module UART thứ 2 của board
#define UART_MODULE (UART_NUM_2)

#define BUF_SIZE (1024)

static void init(void)
{
    /* Configure parameters of an UART driver,
     * communication pins and install the driver */
    uart_config_t uart_config = {
        .baud_rate = UART_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    // Gọi hàm bên trong ESP_ERROR_CHECK() giống như try..catch để abort() chương trình và log error
    // Install driver UART, tạo buffer cho RX
    ESP_ERROR_CHECK(uart_driver_install(UART_MODULE, BUF_SIZE * 2, 0, 0, NULL, 0));
    // Set config đã tạo ở trên cho UART module
    ESP_ERROR_CHECK(uart_param_config(UART_MODULE, &uart_config));
    // Set 2 pin RX, TX cho module (còn 2 pin RTS, CTS giữ nguyên)
    ESP_ERROR_CHECK(uart_set_pin(UART_MODULE, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

static void tx_task(void *arg){
  char *TxData = (char*) malloc(30);

  while (1){
    sprintf(TxData, "Hello world %d \r\n", 100);
    // Transmit data qua UART, gồm nội dung gửi và length
    // Các task TX sẽ block task đến khi toàn bộ data được gửi đi hết
    uart_write_bytes(UART_MODULE, TxData, strlen(TxData));
    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

static void rx_task(void *arg){
  // Set tag để log
  static const char *RX_TASK_TAG = "RX_TASK";
  esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);

  // Tạo biến để nhận data
  uint8_t *data = (uint8_t *) malloc(BUF_SIZE + 1);

  while (1){
    // Đọc data từ UART, set thời gian chờ tối đa, return số byte nhận được
    const int rxBytes = uart_read_bytes(UART_MODULE, data, BUF_SIZE, 500 / portTICK_PERIOD_MS);
    if (rxBytes > 0){
        data[rxBytes] = '\0'; // Thêm character kết thúc cho data
        // Log kết quả
        ESP_LOGI(RX_TASK_TAG, "Read %d bytes: %s", rxBytes, (char *) data);
    }
  }
}

void app_main(void)
{
    init();

    xTaskCreate(rx_task, "uart_rx_task", BUF_SIZE * 2, NULL, configMAX_PRIORITIES, NULL);
    xTaskCreate(tx_task, "uart_tx_task", BUF_SIZE * 2, NULL, configMAX_PRIORITIES - 1, NULL);
}
