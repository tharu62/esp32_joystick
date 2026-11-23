#include "rclink.h"
#include "rclink_protocol.h"

#include <string.h>

#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define TX_INTERVAL_MS 20

static const char *TAG = "RCLINK";

static rclink_state_t state = RCLINK_STATE_DISCOVERING;

static rclink_channels_t channels =
{
    .ch = {1500, 1500, 1000, 1500, 1000, 1000, 1000, 1000}
};

static uint8_t fc_mac[6];

static volatile bool pair_received = false;
static volatile bool alive_received = false;

static uint8_t pair_channel;

static uint32_t next_discovery;
static uint32_t last_alive;
static uint32_t next_transmission;


// ESP-NOW receive callback
static void rclink_rx_callback(
    const esp_now_recv_info_t *info,
    const uint8_t *data,
    int len)
{
    if (info == NULL || data == NULL || len < 2)
        return;

    if (rclink_checksum(data, len - 1) != data[len - 1])
        return;

    switch (data[0])
    {
        case RCLINK_PAIR_REQ:
        {
            if (state != RCLINK_STATE_DISCOVERING)
                return;

            if (len != sizeof(rclink_message_pair_req_t))
                return;

            const rclink_message_pair_req_t *msg =
                (const rclink_message_pair_req_t *)data;

            if (msg->channel < RCLINK_CHANNEL_MIN ||
                msg->channel > RCLINK_CHANNEL_MAX)
                return;

            memcpy(fc_mac, info->src_addr, 6);

            pair_channel = msg->channel;
            pair_received = true;

            break;
        }

        case RCLINK_FC_ALIVE:
        {
            if (state != RCLINK_STATE_TRANSMITTING)
                return;

            if (len != sizeof(rclink_message_alive_t))
                return;

            if (memcmp(info->src_addr, fc_mac, 6) == 0)
                alive_received = true;

            break;
        }

        default:
            break;
    }
}


// Initialize ESP-NOW
static void rclink_espnow_init(void)
{
    ESP_ERROR_CHECK(esp_now_init());

    ESP_ERROR_CHECK(
        esp_now_register_recv_cb(rclink_rx_callback)
    );
}


// Change Wi-Fi channel
static void rclink_set_wifi_channel(uint8_t channel)
{
    ESP_ERROR_CHECK(
        esp_wifi_set_channel(
            channel,
            WIFI_SECOND_CHAN_NONE
        )
    );
}


// Pair with flight controller
static void rclink_pair(void)
{
    ESP_LOGI(TAG, "Pair request received");

    // Synchronize with receiver channel
    rclink_set_wifi_channel(pair_channel);

    // Register FC as ESP-NOW peer
    esp_now_peer_info_t peer = {0};

    memcpy(peer.peer_addr, fc_mac, 6);

    peer.channel = pair_channel;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;

    esp_err_t err = esp_now_add_peer(&peer);

    if (err != ESP_OK && err != ESP_ERR_ESPNOW_EXIST)
    {
        ESP_LOGE(TAG, "Failed to add peer: %s",
                 esp_err_to_name(err));
        return;
    }

    // Construct pairing response
    rclink_message_pair_res_t msg =
    {
        .type = RCLINK_PAIR_RES
    };

    msg.csum = rclink_checksum(
        (const uint8_t *)&msg,
        sizeof(msg) - 1
    );

    // Send pairing response
    err = esp_now_send(
        fc_mac,
        (const uint8_t *)&msg,
        sizeof(msg)
    );

    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Pair response failed: %s",
                 esp_err_to_name(err));
        return;
    }

    state = RCLINK_STATE_TRANSMITTING;

    last_alive = esp_timer_get_time() / 1000;

    ESP_LOGI(TAG, "Paired with FC");
}


// Construct and send RC packet
static void rclink_send_channels(void)
{
    rclink_message_rc_t msg =
    {
        .type = RCLINK_RC_DATA,

        .ch1 = channels.ch[0],
        .ch2 = channels.ch[1],
        .ch3 = channels.ch[2],
        .ch4 = channels.ch[3],

        .ch5 = rclink_encode_aux(channels.ch[4]),
        .ch6 = rclink_encode_aux(channels.ch[5]),
        .ch7 = rclink_encode_aux(channels.ch[6]),
        .ch8 = rclink_encode_aux(channels.ch[7])
    };

    msg.csum = rclink_checksum(
        (const uint8_t *)&msg,
        sizeof(msg) - 1
    );

    esp_err_t err = esp_now_send(
        fc_mac,
        (const uint8_t *)&msg,
        sizeof(msg)
    );

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "RC transmission error: %s",
                 esp_err_to_name(err));
    }
}


// Public initialization
void rclink_init(void)
{
    rclink_espnow_init();

    next_discovery = 0;
    next_transmission = 0;
}


// Communication update
void rclink_update(void)
{
    uint32_t now = esp_timer_get_time() / 1000;

    // Discovery state
    if (state == RCLINK_STATE_DISCOVERING)
    {
        if (pair_received)
        {
            pair_received = false;
            rclink_pair();
            return;
        }

        if ((int32_t)(now - next_discovery) >= 0)
        {
            static uint8_t channel = RCLINK_CHANNEL_MIN;

            channel++;

            if (channel > RCLINK_CHANNEL_MAX)
                channel = RCLINK_CHANNEL_MIN;

            rclink_set_wifi_channel(channel);

            next_discovery = now + RCLINK_DISCOVERY_MS;
        }

        return;
    }

    // Transmission state
    if (state == RCLINK_STATE_TRANSMITTING)
    {
        if (alive_received)
        {
            alive_received = false;
            last_alive = now;
        }

        if ((int32_t)(now - next_transmission) >= 0)
        {
            next_transmission = now + TX_INTERVAL_MS;

            rclink_send_channels();
        }
    }
}


// Update RC channels
void rclink_set_channels(const rclink_channels_t *new_channels)
{
    if (new_channels == NULL)
        return;

    for (int i = 0; i < 8; i++)
    {
        int value = new_channels->ch[i];

        if (value < RCLINK_PWM_MIN)
            value = RCLINK_PWM_MIN;

        if (value > RCLINK_PWM_MAX)
            value = RCLINK_PWM_MAX;

        channels.ch[i] = value;
    }
}


rclink_state_t rclink_get_state(void)
{
    return state;
}


uint32_t rclink_get_alive_age(void)
{
    return (esp_timer_get_time() / 1000) - last_alive;
}
