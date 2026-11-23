#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "state.h"
#include "espnow.h"
#include "buttons.h"
#include "stick.h"
#include <stdio.h>

#define TAG "JOY_MASTER"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 Joystick Master started");
    
    // adc_init_throttle(); // throttle only function used for testing only
    init_adc_all();
    // espnow_standard_init(); // old standard implementation without rclink
    espnow_rclink_init();
    // init_buttons_interrupts();

    /* task that implementes the data comunication throught espnow */
    // xTaskCreate(
    //     espnow_send_data,
    //     "espnow_send_data",
    //     4096,
    //     NULL,
    //     5,
    //     NULL
    // );

    // while(true){
    //     ESP_LOGI(TAG, "test");
    //     vTaskDelay(pdMS_TO_TICKS(1000));  // = 1 secondo, indipendentemente dal tick rate   
    // }   
    
    xTaskCreate(
        test_func,
        "test_func",
        4096,
        NULL,
        5,
        NULL
    );
}
