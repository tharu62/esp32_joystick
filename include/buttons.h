#pragma once

#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"

#include "state.h"

void init_buttons(void);

void init_buttons_interrupts();