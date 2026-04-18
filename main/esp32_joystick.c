#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "state.h"
#include "espnow.h"
#include "buttons.h"
#include "thumbstick.h"

#define TAG "JOY_MASTER"

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 Joystick Master started");
    
    calibration_state cs;

    // adc_init_throttle(); // throttle only function used for testing only
    init_adc_all();
    calibrate_adc(&cs);
    espnow_init(&cs);
    init_buttons_interrupts();

    xTaskCreate(
        espnow_send_data,
        "espnow_send_data",
        4096,
        NULL,
        5,
        NULL
    );
}
