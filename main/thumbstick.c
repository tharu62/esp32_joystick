#include "thumbstick.h"

#define TAG "JOY_MASTER"

// ESP32-S3 ADC
#define ADC_UNIT              ADC_UNIT_1
#define THROTTLE_ADC_CHANNEL  ADC_CHANNEL_3   // GPIO04
#define ROLL_ADC_CHANNEL      ADC_CHANNEL_0   // GPIO1
#define PITCH_ADC_CHANNEL     ADC_CHANNEL_1   // GPIO02
#define YAW_ADC_CHANNEL       ADC_CHANNEL_4   // GPIO05
#define ADC_MAX               4095.0f

static adc_oneshot_unit_handle_t adc_handle;

void adc_init_throttle(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,   // <-- CORRECT FOR IDF 5.x
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        THROTTLE_ADC_CHANNEL,
        &chan_cfg
    ));

    // ESP_LOGI(TAG, "ADC initialized (GPIO10)");
}

void init_adc_all(void)
{
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };
    
    // Configure throttle channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        THROTTLE_ADC_CHANNEL,
        &chan_cfg
    ));
    
    // Configure roll channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        ROLL_ADC_CHANNEL,
        &chan_cfg
    ));
    
    // Configure pitch channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        PITCH_ADC_CHANNEL,
        &chan_cfg
    ));

    // Configure yaw channel
    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        YAW_ADC_CHANNEL,
        &chan_cfg
    ));
    
    ESP_LOGI(TAG, "ADC initialized for all channels");
}

float read_throttle(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, THROTTLE_ADC_CHANNEL, &raw);
    if (raw < 0) raw = 0;
    if (raw > ADC_MAX) raw = ADC_MAX;
    // Throttle is wired inverted: forward is lower voltage.
    float t = 1.0f - raw / ADC_MAX;
    return t;
}

float read_roll(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, ROLL_ADC_CHANNEL, &raw);
    if (raw < 0) raw = 0;
    if (raw > ADC_MAX) raw = ADC_MAX;
    float r = 1.0f - (raw / ADC_MAX);
    return r;
}

float read_pitch(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, PITCH_ADC_CHANNEL, &raw);
    if (raw < 0) raw = 0;
    if (raw > ADC_MAX) raw = ADC_MAX;
    float p = (raw / ADC_MAX);
    return p;
}

float read_yaw(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, YAW_ADC_CHANNEL, &raw);
    if (raw < 0) raw = 0;
    if (raw > ADC_MAX) raw = ADC_MAX;    
    float y = (raw / ADC_MAX);
    return y;
}


