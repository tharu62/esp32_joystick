#include "espnow.h"
#include "freertos/FreeRTOS.h"

#define TAG "ESPNOW"

typedef struct __attribute__((packed))
{
    float throttle;
    float roll;
    float pitch;
    float yaw;
} message;

#define SEND_PERIOD_MS 50 // 20 Hz
#define GAIN 2.0f 

// CHANGE THIS TO THE DRONE MAC ADDRESS
uint8_t drone_mac[ESP_NOW_ETH_ALEN] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };

static void espnow_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
    //ESP_LOGD(TAG, "Send status: %s", status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

/**
 * Initialize ESP-NOW protocol for communication with drone
 */
void espnow_init()
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
    ESP_LOGI("", "ESP-NOW initialized");

}

/**
 * Clamp small joystick values to zero to prevent drift due to noise.
 * This creates a deadzone around the center position of the joystick.
 */
float clamp(float value){
    if(value > -0.15f && value < 0.15f){
        return 0.f;        
    }
    else if(value < -1.0f){
        return -1.0f;
    } 
    else if(value > 1.0f){
        return 1.0f;
    }
    else return value;
}

float clamp_throttle(float value){
    if(value < 0.15f && value > -0.8f){
        value = 0.0f;
    } 
    else if(value > 1.0f){
        value = 1.0f;
    }
    return value;
}

/**
 * Task to read joystick data and send via ESP-NOW
 * @todo adding button states to the message struct and sending them as well
 */
void espnow_send_data(void *arg)
{
    message msg = {0};

    while (1)
    {
        msg.throttle = clamp_throttle((read_throttle() - 0.5f) * GAIN); 
        msg.roll     = 40.0f * clamp((read_roll()     - 0.5f) * GAIN);
        msg.pitch    = 40.0f * clamp((read_pitch()    - 0.5f) * GAIN);
        // msg.yaw      = 90.0f * clamp((read_yaw()      - 0.5f) * GAIN);
        msg.yaw      = 0.0f; // Yaw control is not implemented in this example
        
        esp_now_send(drone_mac, (uint8_t *)&msg, sizeof(msg));
        ESP_LOGI(TAG, "[%lu] Throttle: %.3f Roll: %.3f Pitch: %.3f Yaw: %.3f", xTaskGetTickCount(), msg.throttle, msg.roll, msg.pitch, msg.yaw);
        vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
    }
}