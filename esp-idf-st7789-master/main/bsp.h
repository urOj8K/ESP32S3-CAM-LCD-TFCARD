#pragma once

#include "esp_system.h"

#define LED_CTR_PIN     1
#define LED_ON      		gpio_set_level( GPIO_RESET, 1 )
#define LED_OFF      		gpio_set_level( GPIO_RESET, 0 )


void sys_key_handler(uint16_t adc_in);
void led_pwm_ctrl_config(void);

extern uint8_t get_a_picture_flag;

