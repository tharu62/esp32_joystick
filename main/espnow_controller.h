#pragma once

#include "esp_wifi.h"
#include "esp_now.h"

/**
 * Initialize ESP-NOW protocol for communication with drone
 */
void espnow_init(void);

/**
 * Clamp small joystick values to zero to prevent drift due to noise.
 * This creates a deadzone around the center position of the joystick.
 */
float clamp(float value);

/**
 * Task to read joystick data and send via ESP-NOW
 * @todo adding button states to the message struct and sending them as well
 */
void espnow_send_data(void *arg);