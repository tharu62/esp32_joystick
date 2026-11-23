#pragma once

#include <stdint.h>
#include <stddef.h>

#define RCLINK_CHANNEL_MIN       1
#define RCLINK_CHANNEL_MAX       13
#define RCLINK_DEFAULT_CHANNEL   7

#define RCLINK_PWM_MIN           880
#define RCLINK_PWM_CENTER        1500
#define RCLINK_PWM_MAX           2120

#define RCLINK_DISCOVERY_MS      200
#define RCLINK_BEACON_MS         50
#define RCLINK_ALIVE_MS          1000

typedef enum
{
    RCLINK_RC_DATA  = 0x01,
    RCLINK_FC_ALIVE = 0x10,
    RCLINK_FC_DATA  = 0x11,
    RCLINK_PAIR_REQ = 0xFE,
    RCLINK_PAIR_RES = 0xFF

} rclink_message_type_t;

typedef struct __attribute__((packed))
{
    uint8_t type;

    int16_t ch1;
    int16_t ch2;
    int16_t ch3;
    int16_t ch4;

    int8_t ch5;
    int8_t ch6;
    int8_t ch7;
    int8_t ch8;

    uint8_t csum;

} rclink_message_rc_t;

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint8_t csum;

} rclink_message_alive_t;

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint8_t channel;
    uint8_t csum;

} rclink_message_pair_req_t;

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint8_t csum;

} rclink_message_pair_res_t;


// Protocol checksum
static inline uint8_t rclink_checksum(
    const uint8_t *data,
    size_t len)
{
    uint8_t checksum = 0x55;

    for (size_t i = 0; i < len; i++)
    {
        checksum ^= data[i];
    }

    return checksum;
}


// Encode auxiliary channel
static inline int8_t rclink_encode_aux(int value)
{
    if (value < RCLINK_PWM_MIN)
        value = RCLINK_PWM_MIN;

    if (value > RCLINK_PWM_MAX)
        value = RCLINK_PWM_MAX;

    value -= RCLINK_PWM_CENTER;

    int round = value > 0 ? 2 : -2;

    return (int8_t)((value + round) / 5);
}


// Decode auxiliary channel
static inline uint16_t rclink_decode_aux(int8_t value)
{
    int result = RCLINK_PWM_CENTER + (value * 5);

    if (result < RCLINK_PWM_MIN)
        result = RCLINK_PWM_MIN;

    if (result > RCLINK_PWM_MAX)
        result = RCLINK_PWM_MAX;

    return (uint16_t)result;
}
