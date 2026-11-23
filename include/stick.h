#pragma once

#include "esp_log.h"
#include "esp_err.h"

#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_event.h"

#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"
#include "state.h"

extern float THROTTLE_ERROR;
extern float ROLL_ERROR;
extern float PITCH_ERROR;


/**
 * Initialize ADC for throttle, auxiliary function for error checking (GPIO04)
 */
void adc_init_throttle(void);

/**
 * Initialize ADC for all channels (throttle, roll, pitch, yaw)
 */
void init_adc_all(void);

int read_throttle(void);

int read_roll(void);

int read_pitch(void);

int read_yaw(void);

uint16_t map_stick(int raw,int adc_min,int adc_max);