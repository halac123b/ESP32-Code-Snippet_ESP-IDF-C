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


#define BUTTON_PIN GPIO_NUM_0 //coi GPIO_NUM_May la button thi dung cai nay e Son dang xai chan 0

void print_mssv(void* parameter) {
    while(1) {
        printf("MSSV: 2011981\n");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void button(void* parameter) {
    while(1) {
        if(gpio_get_level(BUTTON_PIN) == 0) {
            printf("ESP32\n");
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    printf("Hello world!");
    xTaskCreate(print_mssv, "print_mssv", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    //config the GPIO pin for button
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = 1ULL << BUTTON_PIN;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ONLY;
    gpio_config(&io_conf);
    xTaskCreate(button, "button", configMINIMAL_STACK_SIZE, NULL, 2, NULL);
}
