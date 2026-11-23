#pragma once

#include <stdint.h>
#include <stdbool.h>

typedef enum
{
    RCLINK_STATE_DISCOVERING,
    RCLINK_STATE_TRANSMITTING

} rclink_state_t;

typedef struct
{
    uint16_t ch[8];

} rclink_channels_t;

void rclink_init(void);

void rclink_update(void);

void rclink_set_channels(const rclink_channels_t *channels);

rclink_state_t rclink_get_state(void);

uint32_t rclink_get_alive_age(void);
