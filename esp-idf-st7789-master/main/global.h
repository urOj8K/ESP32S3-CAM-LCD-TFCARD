#ifndef global_H
#define global_H

// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "freertos/event_groups.h"
// #include "freertos/semphr.h"

// #include "esp_system.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "esp_bt.h"
// #include "esp_gap_ble_api.h"
// #include "esp_gatts_api.h"
// #include "esp_bt_defs.h"
// #include "esp_bt_main.h"
// #include "esp_gatt_common_api.h"
// #include "esp_timer.h"
// #include "esp_adc_cal.h"

// #include "sdkconfig.h"

// #include "driver/gpio.h"
// #include "driver/spi_master.h"
// #include "driver/adc.h"


// #include "lkn_spi.h"
// #include "lkn_oled.h"
// #include "lkn_font.h"
// #include "lkn_adc.h"


#ifdef _MAIN_
#define GLOBAL_EXTERN 
#else
#define GLOBAL_EXTERN  extern
#endif

#define CONFIG_OV7670_SUPPORT 0
#define CONFIG_OV7725_SUPPORT 0
#define CONFIG_NT99141_SUPPORT 0
#define CONFIG_OV2640_SUPPORT 1
#define CONFIG_OV3660_SUPPORT 0
#define CONFIG_OV5640_SUPPORT 0
#define CONFIG_GC2145_SUPPORT 0
#define CONFIG_GC032A_SUPPORT 0
#define CONFIG_GC0308_SUPPORT 0
#define CONFIG_BF3005_SUPPORT 0
#define CONFIG_BF20A6_SUPPORT 0
#define CONFIG_SC030IOT_SUPPORT 0
#define CONFIG_SCCB_HARDWARE_I2C_PORT1 1
#define CONFIG_SCCB_CLK_FREQ 100000
#define CONFIG_GC_SENSOR_SUBSAMPLE_MODE 1
#define CONFIG_CAMERA_CORE0 1
#define CONFIG_CAMERA_DMA_BUFFER_SIZE_MAX 32768


// typedef struct
// {
// 	uint16_t usx1;
// 	uint16_t usx4;
// 	uint16_t usx16;
// 	uint16_t vbat;
// } _adcValue;

// GLOBAL_EXTERN 	_adcValue  adcValue;
#endif