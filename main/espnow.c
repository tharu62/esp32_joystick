#include "freertos/FreeRTOS.h"
#include "espnow.h"
#include "stick.h"

#define TAG "ESPNOW"

#define RCLINK_DEFAULT_CHANNEL 7
#define SEND_PERIOD_MS 50 // 20 Hz


// OLD COMUNICATION PACKET STRUCTURE WITHOUT RCLINK
typedef struct __attribute__((packed)){
    float throttle;
    float roll;
    float pitch;
    float yaw;
} message;

// CHANGE THIS TO THE DRONE MAC ADDRESS (FOR IMPLEMENTATION WITHOUT RCLINK)
uint8_t drone_mac[ESP_NOW_ETH_ALEN] = { 0x02, 0x00, 0x00, 0x00, 0x00, 0x01 };


static void espnow_send_cb(const esp_now_send_info_t *tx_info, esp_now_send_status_t status)
{
    // OPTIONAL ERROR LOG 
    //ESP_LOGD(TAG, "Send status: %s", status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

static void espnow_recv_cb(const esp_now_recv_info_t *rx_info,const uint8_t *data,int len){
    
}


/**
 * Initialize ESP-NOW protocol for communication with drone
 */
void espnow_standard_init()
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

    // FOR IMPLEMENTATION WITHOUT RCLINK
    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, drone_mac, ESP_NOW_ETH_ALEN);
    peer.channel = 0;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));
    
    ESP_LOGI("", "ESP-NOW initialized");
}

void espnow_rclink_init(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    ESP_ERROR_CHECK(ret);

    // Initialize network interface
    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize Wi-Fi
    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(
        esp_wifi_set_channel(
            RCLINK_DEFAULT_CHANNEL,
            WIFI_SECOND_CHAN_NONE
        )
    );
    
    // Initialize ESP-NOW
    ESP_ERROR_CHECK(esp_now_init());

    // Register callbacks
    ESP_ERROR_CHECK(
        esp_now_register_send_cb(espnow_send_cb)
    );

    ESP_ERROR_CHECK(
        esp_now_register_recv_cb(espnow_recv_cb)
    );

    ESP_LOGI(TAG, "ESP-NOW initialized");
}


/**
 * Clamp small joystick values to zero to prevent drift due to noise.
 */
float clamp(float value){
    if(value < 0.15){
        return 0.f;        
    }
    if(value > 1.0f){
        return 1.0f;
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
        msg.throttle = clamp(read_throttle()); 
        msg.roll     = clamp(read_roll()    );
        msg.pitch    = clamp(read_pitch()   );
        msg.yaw      = clamp(read_yaw()     );
        
        esp_now_send(drone_mac, (uint8_t *)&msg, sizeof(msg));
        ESP_LOGI(TAG, "[%lu] Throttle: %.3f Roll: %.3f Pitch: %.3f Yaw: %.3f", xTaskGetTickCount(), msg.throttle, msg.roll, msg.pitch, msg.yaw);
        vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
    }
}

void test_func(void *arg)
{
        while (1)
    {
        ESP_LOGI(TAG, "[%lu] Throttle: %f Roll: %f Pitch: %f Yaw: %f", read_throttle(), read_roll(), read_pitch(), read_yaw());
        // ESP_LOGI(TAG, "Throttle : %f", read_throttle());
        vTaskDelay(pdMS_TO_TICKS(SEND_PERIOD_MS));
    }
}