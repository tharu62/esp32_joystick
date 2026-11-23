#pragma once

#include <string.h>
#include <math.h>

#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "esp_wifi.h"
#include "esp_now.h"

#include "stick.h"
#include "state.h"


/**
 * Initialize ESP-NOW standard protocol for communication with drone
 */
void espnow_standard_init();

/**
 * Initialize ESP-NOW protocol with rclink for communication with drone
 */
void espnow_rclink_init();

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

void test_func(void *arg);