#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "esp_wifi.h"
#include "esp_now.h"

#include "esp_adc/adc_oneshot.h"

#include "driver/gpio.h"

/* ===================== CONFIG ===================== */

#define TAG "JOY_MASTER"

// ESP32-S3 ADC
#define ADC_UNIT              ADC_UNIT_1
#define THROTTLE_ADC_CHANNEL  ADC_CHANNEL_9   // GPIO10
#define ROLL_ADC_CHANNEL      ADC_CHANNEL_1   // GPIO2
#define PITCH_ADC_CHANNEL     ADC_CHANNEL_0   // GPIO1
#define ADC_MAX               4095.0f

#define SEND_PERIOD_MS        20   // 50 Hz

// CHANGE THIS TO YOUR DRONE MAC
uint8_t drone_mac[ESP_NOW_ETH_ALEN] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };

/* ===================== DATA ===================== */

typedef struct __attribute__((packed))
{
    float throttle;
    float roll;
    float pitch;
} message;

/* ===================== ADC ===================== */

static adc_oneshot_unit_handle_t adc_handle;

/**
 * Initialize ADC for throttle, AUXILIARY FUNCTION FOR ERROR CHECKING(GPIO10)
 */
static void adc_init_throttle(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,   // <-- CORRECT FOR IDF 5.x
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        THROTTLE_ADC_CHANNEL,
        &chan_cfg
    ));

    ESP_LOGI(TAG, "ADC initialized (GPIO10)");
}

/**
 * Initialize ADC for all channels (throttle, roll, pitch)
 */
void init_adc_all(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    
    // Configure throttle channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        THROTTLE_ADC_CHANNEL,
        &chan_cfg
    ));
    
    // Configure roll channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        ROLL_ADC_CHANNEL,
        &chan_cfg
    ));
    
    // Configure pitch channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        PITCH_ADC_CHANNEL,
        &chan_cfg
    ));
    
    ESP_LOGI(TAG, "ADC initialized for all channels");
}

static float read_throttle(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, THROTTLE_ADC_CHANNEL, &raw);

    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;

    float t = raw / ADC_MAX;

    // Noise clamp
    if (t < 0.02f)
        t = 0.0f;

    return t;
}

static float read_roll(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, ROLL_ADC_CHANNEL, &raw);

    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;

    float r = raw / ADC_MAX;

    // Noise clamp
    if (r < 0.02f)
        r = 0.0f;

    return r;
}

static float read_pitch(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, PITCH_ADC_CHANNEL, &raw);

    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;

    float p = raw / ADC_MAX;

    // Noise clamp
    if (p < 0.02f)
        p = 0.0f;

    return p;
}

/* ===================== ESP-NOW ===================== */

// ✅ CORRECT CALLBACK SIGNATURE (IDF 5.x)
static void espnow_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
    ESP_LOGD(TAG, "Send status: %s", status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

/**
 * Initialize ESP-NOW protocol for communication with drone
 */
static void espnow_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_cb));

    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, drone_mac, ESP_NOW_ETH_ALEN);
    peer.channel = 0;
    peer.encrypt = false;

    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    ESP_LOGI(TAG, "ESP-NOW initialized");
}

/* ===================== TASK ===================== */


/**
 * Task to read joystick data and send via ESP-NOW
 */
static void espnow_send_data(void *arg)
{
    message msg = {0};

    while (1)
    {
        msg.throttle = read_throttle();
        msg.roll  = read_roll();
        msg.pitch = read_pitch();

        esp_now_send(drone_mac, (uint8_t *)&msg, sizeof(msg));

        ESP_LOGI(TAG, "Throttle: %.2f Roll: %.2f Pitch: %.2f", msg.throttle, msg.roll, msg.pitch);

        vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
    }
}

/* ===================== MAIN ===================== */

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-S3 Joystick Master");

    // adc_init_throttle();
    init_adc_all();
    espnow_init();

    xTaskCreate(
        espnow_send_data,"espnow_send_data",4096,NULL,5,NULL
    );
}
