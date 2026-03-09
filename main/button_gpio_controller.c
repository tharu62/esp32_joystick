#include "button_gpio_controller.h"
#include <stdint.h>

#define BUTTON_JOY_X GPIO_NUM_10
#define BUTTON_JOY_Y GPIO_NUM_11
#define BUTTON_JOY_A GPIO_NUM_12
#define BUTTON_JOY_B GPIO_NUM_13

volatile bool button_x_pressed = false;
volatile bool button_y_pressed = false;
volatile bool button_a_pressed = false;
volatile bool button_b_pressed = false;


void IRAM_ATTR button_pressed(void *arg)
{
    char button_id = (char)(uintptr_t)arg;
    switch (button_id) {
        case 'X':
            button_x_pressed = true;
            break;
        case 'Y':
            button_y_pressed = true;
            break;
        case 'A':
            button_a_pressed = true;
            break;
        case 'B':
            button_b_pressed = true;
            break;
    }
    return;
}

void init_buttons_gpio_interrupts()
{

    gpio_set_direction(BUTTON_JOY_A, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_JOY_A, GPIO_PULLUP_ENABLE);

    gpio_set_intr_type(BUTTON_JOY_A, GPIO_INTR_HIGH_LEVEL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_JOY_A, button_pressed, (void *)'A');

    gpio_set_direction(BUTTON_JOY_B, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_JOY_B, GPIO_PULLUP_ENABLE);

    gpio_set_intr_type(BUTTON_JOY_B, GPIO_INTR_HIGH_LEVEL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_JOY_B, button_pressed, (void *)'B');

    gpio_set_direction(BUTTON_JOY_X, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_JOY_X, GPIO_PULLUP_ENABLE);

    gpio_set_intr_type(BUTTON_JOY_X, GPIO_INTR_HIGH_LEVEL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_JOY_X, button_pressed, (void *)'X');

    gpio_set_direction(BUTTON_JOY_Y, GPIO_MODE_INPUT);
    gpio_set_pull_mode(BUTTON_JOY_Y, GPIO_PULLUP_ENABLE);

    gpio_set_intr_type(BUTTON_JOY_Y, GPIO_INTR_HIGH_LEVEL);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_JOY_Y, button_pressed, (void *)'Y');
}