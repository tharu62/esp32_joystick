#pragma once

typedef struct{
    float throttle;
    float roll;
    float pitch;
    float yaw;
}calibration_state;

typedef struct{
    bool button_x_pressed;
    bool button_y_pressed;
    bool button_a_pressed;
    bool button_b_pressed;
}buttons_state;