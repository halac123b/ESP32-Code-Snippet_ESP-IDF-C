#include <stdio.h>
#include <inttypes.h>

#include <FreeRTOSConfig.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Lib to connect with GPIO pins
#include <driver/gpio.h>

// Connect Button with pin GPIO_17
#define BUTTON_PIN GPIO_NUM_17

// Each function stands for a task in OS
void printMssv(void* parameter) {
    while(1) {
        printf("MSSV: 2011130\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

// Init GPIO pin
void initButton(int btnPin) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << btnPin);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;

    gpio_config(&io_conf);
}

// 2nd task take input signal from button
void button(void* parameter) {
    initButton(BUTTON_PIN);
    while(1) {
        // Function to get gpio signal
        if(gpio_get_level(BUTTON_PIN) == 0) {
            printf("Detect button: ESP32\n");
            // Add some delay to prevent button signal noise
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    printf("Hello world!");

    // Start 2 task cùng 1 lúc
    xTaskCreate(printMssv, "printMssv", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(button, "button", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}