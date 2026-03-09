#pragma once

#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

extern float THROTTLE_ERROR;
extern float ROLL_ERROR;
extern float PITCH_ERROR;

/**
 * Initialize ADC for throttle, AUXILIARY FUNCTION FOR ERROR CHECKING(GPIO10)
 */
void adc_init_throttle(void);

/**
 * Initialize ADC for all channels (throttle, roll, pitch)
 */
void init_adc_all(void);

float read_throttle(void);

float read_roll(void);

float read_pitch(void);

/**
 * Calibrate ADC by averaging multiple readings to find the 
 * error offsets for throttle, roll, and pitch. 
 * This ensures that the joystick readings are centered around zero when at rest.
 */
void calibrate_adc();
