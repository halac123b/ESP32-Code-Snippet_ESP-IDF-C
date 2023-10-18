/*
 * SPDX-FileCopyrightText: 2010-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include <FreeRTOSConfig.h>
#include <driver/gpio.h>


#define TOUCH_GPIO_NUM 13
#define RESET_BUTTON_GPIO_NUM 0

void initialize_reset_button() {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << RESET_BUTTON_GPIO_NUM);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; // or GPIO_PULLDOWN_ENABLE if needed
    gpio_config(&io_conf);
}

void monitor_reset_button(void* parameter) {
    while(1) {
        if (gpio_get_level(RESET_BUTTON_GPIO_NUM) == 0) {
            printf("ESP32\n");
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}


void print_mssv(void* parameter) {
    while(1) {
        printf("MSSV: 2011981\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}


// void touch(void* parameter) {
//     esp_rom_gpio_pad_select_gpio(TOUCH_GPIO_NUM);
//     gpio_set_direction(TOUCH_GPIO_NUM, GPIO_MODE_INPUT);
//     gpio_set_pull_mode(TOUCH_GPIO_NUM, GPIO_PULLUP_ONLY);
//     while(1) {
//         if (gpio_get_level(TOUCH_GPIO_NUM) == 0) {
//             printf("ESP32\n");
//         }
//         vTaskDelay(100 / portTICK_PERIOD_MS);
//     }
// }

void app_main() {
    xTaskCreate(print_mssv, "print_mssv", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    // xTaskCreate(touch, "touch", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    initialize_reset_button();
    xTaskCreate(monitor_reset_button, "monitor_reset_button", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    
}