#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "thumbstick_adc_controller.h"
#include "espnow_controller.h"
#include "button_gpio_controller.h"

#define TAG "JOY_MASTER"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 Joystick Master");

    // adc_init_throttle();
    init_adc_all();
    calibrate_adc();
    espnow_init();
    init_buttons_gpio_interrupts();

    xTaskCreate(
        espnow_send_data,
        "espnow_send_data",
        4096,
        NULL,
        5,
        NULL
    );
}
