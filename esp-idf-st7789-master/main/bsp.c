#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "xclk.h"
#include "esp_camera.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "bsp.h"

static const char *TAG = "BSP";

uint8_t get_a_picture_flag = 0;

#define  KEY_NONE   0
#define  KEY_SW1    1
#define  KEY_SW2    2
#define  KEY_SW3    3
#define  KEY_SW4    4
#define  KEY_SW5    5

uint16_t key_buf[] = {1400,1800,2400,2900,3300,4000};

#define TARGET_MCPWM_UNIT MCPWM_UNIT_0
#define TIMER0_OUTPUT_GPIO GPIO_NUM_1

static uint8_t led_ctrl_duty = 1;

void sys_gpio_config()
{
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[LED_CTR_PIN], PIN_FUNC_GPIO);
    gpio_set_direction(LED_CTR_PIN, GPIO_MODE_OUTPUT);
    gpio_set_pull_mode(LED_CTR_PIN, GPIO_FLOATING);
    gpio_matrix_out(LED_CTR_PIN, CAM_CLK_IDX, false, false);
}

uint8_t key_index(uint16_t adc_in)
{
    for(int i=0;i<6;i++)
    {
        if(adc_in<key_buf[i])
            return i;
    }
    return 0;
}

uint8_t read_key_value(uint16_t adc_in)
{
    static uint8_t key_down_delay = 0;
    static uint8_t key_down_sta = 0;
    static uint8_t old_key_value = 0;
    uint8_t key_num=key_index(adc_in);

    if(key_down_sta == 0)
    {
        if(!key_num)
            return 0;
        else
        {
            if(++key_down_delay>5)
            {
                key_down_delay = 0;
                key_down_sta = 1;
                old_key_value = key_num;
            }
            return 0;
        }
    }
    else
    {
        if(key_num)
            return 0;
        else
        {
            key_down_sta = 0;
            return(old_key_value);
        }
    }
}

void sys_key_handler(uint16_t adc_in)
{
    uint8_t key_num=read_key_value(adc_in);
    
    if(!key_num)
        return;
    switch (key_num)
    {
        case KEY_SW1:
            get_a_picture_flag = 1;
            break;
        case KEY_SW2:
            break;
        case KEY_SW3:
            break;
        case KEY_SW4:
            break;
        case KEY_SW5:
            break;
        default:
            break;
    }
}


void led_pwm_ctrl_config()
{
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, TIMER0_OUTPUT_GPIO); // To drive a RC servo, one MCPWM generator is enough

    mcpwm_config_t pwm_config = {
        .frequency = 1000, // frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        .cmpr_a = 0,     // duty cycle of PWMxA = 0
        .counter_mode = MCPWM_UP_COUNTER,
        .duty_mode = MCPWM_DUTY_MODE_0,
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);
    mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, led_ctrl_duty);
}


