#include "thumbstick_adc_controller.h"

#define TAG "JOY_MASTER"

// ESP32-S3 ADC
#define ADC_UNIT              ADC_UNIT_1
#define THROTTLE_ADC_CHANNEL  ADC_CHANNEL_9   // GPIO10
#define ROLL_ADC_CHANNEL      ADC_CHANNEL_1   // GPIO2
#define PITCH_ADC_CHANNEL     ADC_CHANNEL_0   // GPIO1
#define ADC_MAX               4095.0f

static adc_oneshot_unit_handle_t adc_handle;
float THROTTLE_ERROR = 0.f;
float ROLL_ERROR = 0.f;
float PITCH_ERROR = 0.f;

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
    
    // ESP_LOGI(TAG, "ADC initialized for all channels");
}

float read_throttle(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, THROTTLE_ADC_CHANNEL, &raw);

    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;

    float t = raw / ADC_MAX;

    // Noise clamp
    if (t < 0.02f)
        t = 0.0f;

    return t;
}

float read_roll(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, ROLL_ADC_CHANNEL, &raw);

    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;

    float r = raw / ADC_MAX;

    // Noise clamp
    if (r < 0.02f)
        r = 0.0f;

    return r;
}

float read_pitch(void)
{
    int raw = 0;
    adc_oneshot_read(adc_handle, PITCH_ADC_CHANNEL, &raw);

    if (raw < 0) raw = 0;
    if (raw > 4095) raw = 4095;

    float p = raw / ADC_MAX;

    // Noise clamp
    if (p < 0.02f)
        p = 0.0f;

    return p;
}

void calibrate_adc()
{
    for(int i=0; i<2000; i++){
        THROTTLE_ERROR += read_throttle();
        ROLL_ERROR += read_roll();
        PITCH_ERROR += read_pitch();
    }
    THROTTLE_ERROR = THROTTLE_ERROR/ 2000.f;
    ROLL_ERROR = ROLL_ERROR / 2000.f;
    PITCH_ERROR = PITCH_ERROR / 2000.f;
    return;
}

