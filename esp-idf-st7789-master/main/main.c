#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_vfs.h"
#include "esp_spiffs.h"
#include "esp_camera.h"
#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"

#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "sdmmc_cmd.h"
#include "sdkconfig.h"

#include "driver/sdmmc_host.h"

#include "st7789.h"
#include "bmpfile.h"
//#include "decode_jpeg.h"
//#include "decode_png.h"
//#include "pngle.h"

#include "driver/i2c.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"

#include "bsp.h"

#define	INTERVAL		400
#define WAIT	vTaskDelay(INTERVAL)

static const char *TAG = "ST7789";
#define MOUNT_POINT "/sdcard"

#define KEY_ADC_CHANNLE     ADC1_CHANNEL_3

// static void SPIFFS_Directory(char * path) {
// 	DIR* dir = opendir(path);
// 	assert(dir != NULL);
// 	while (true) {
// 		struct dirent*pe = readdir(dir);
// 		if (!pe) break;
// 		ESP_LOGI(__FUNCTION__,"d_name=%s d_ino=%d d_type=%x", pe->d_name,pe->d_ino, pe->d_type);
// 	}
// 	closedir(dir);
// }

// You have to set these CONFIG value using menuconfig.
#if 1
#define CONFIG_WIDTH  360
#define CONFIG_HEIGHT 240
#define CONFIG_MOSI_GPIO 38		//
#define CONFIG_SCLK_GPIO 39		//
#define CONFIG_CS_GPIO 46		//
#define CONFIG_DC_GPIO 0		//
#define CONFIG_RESET_GPIO 6	//
#define CONFIG_BL_GPIO -1
#endif

#define CAM_PIN_PWDN -1  //power down is not used
#define CAM_PIN_RESET -1 //software reset will be performed
#define CAM_PIN_XCLK 18
#define CAM_PIN_SIOD 45
#define CAM_PIN_SIOC 48

#define CAM_PIN_D7 47
//#define CAM_PIN_D7 26
#define CAM_PIN_D6 21
#define CAM_PIN_D5 14
//#define CAM_PIN_D5 33
#define CAM_PIN_D4 13
#define CAM_PIN_D3 11
#define CAM_PIN_D2 9
#define CAM_PIN_D1 10
#define CAM_PIN_D0 12
#define CAM_PIN_HREF 7
#define CAM_PIN_VSYNC 8
#define CAM_PIN_PCLK 17

/**********************************
 * 	SD CARD PINS
 * ********************************/
#define PIN_NUM_MISO 40
#define PIN_NUM_MOSI 38
#define PIN_NUM_CLK  39
#define PIN_NUM_CS   41

#define SPI_DMA_CHAN    SPI_DMA_CH_AUTO
#define SDSPI_HOST_ID  SPI2_HOST

#define SD_CARD

#define CAM_WIDE	320
#define	CAM_HIGH	240

const uint8_t bmp_head_320[]={
0x42,0x4D,0x36,0x84,0x03,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
0x00,0x00,0x40,0x01,0x00,0x00,0xF0,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
0x00,0x00,0x00,0x84,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};

const uint8_t bmp_head_640[]={
0x42,0x4D,0x36,0x10,0x0E,0x00,0x00,0x00,0x00,0x00,0x36,0x00,0x00,0x00,0x28,0x00,
0x00,0x00,0x80,0x02,0x00,0x00,0xE0,0x01,0x00,0x00,0x01,0x00,0x18,0x00,0x00,0x00,
0x00,0x00,0x00,0x10,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00
};

void spi_sd_init()
{
    esp_err_t ret;
	
   esp_vfs_fat_sdmmc_mount_config_t mount_config = {
#ifdef CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .format_if_mount_failed = true,
#else
        .format_if_mount_failed = false,
#endif // EXAMPLE_FORMAT_IF_MOUNT_FAILED
        .max_files = 5,
        .allocation_unit_size = 16 * 1024
    };
    sdmmc_card_t *card;
    const char mount_point[] = MOUNT_POINT;
    ESP_LOGI(TAG, "Initializing SD card");

    // Use settings defined above to initialize SD card and mount FAT filesystem.
    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
    // Please check its source code and implement error recovery when developing
    // production applications.
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    spi_bus_config_t bus_cfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };
    ret = spi_bus_initialize(host.slot, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return;
    }
	

   // This initializes the slot without card detect (CD) and write protect (WP) signals.
    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = PIN_NUM_CS;
    slot_config.host_id = host.slot;

    ESP_LOGI(TAG, "Mounting filesystem");
    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return;
    }
    ESP_LOGI(TAG, "Filesystem mounted");

    sdmmc_card_print_info(stdout, card);
}

int pictureNumber = 0;

void main_task(void *pvParameters)
{
	TFT_t dev;
	uint8_t flag = 0;
	spi_sd_init();
	spi_master_init(&dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO, CONFIG_BL_GPIO);
	lcdInit(&dev, CONFIG_WIDTH, CONFIG_HEIGHT, CONFIG_OFFSETX, CONFIG_OFFSETY);

	while (1) {
		camera_fb_t *pic = esp_camera_fb_get();
		// ESP_LOGI(TAG, "Picture taken! Its size was: %zu bytes", pic->len);
		lcdShowPiture(&dev,pic->buf);
		esp_camera_fb_return(pic);		

#ifdef SD_CARD
		if(get_a_picture_flag)
		{
			get_a_picture_flag = 0;

			//以下代码作用为新建图片文件名序号递增
			const char *pic_cnt = MOUNT_POINT"/piccnt.txt";
			FILE *f0 = fopen(pic_cnt, "r+");

			if (f0 == NULL) {
				ESP_LOGE(TAG, "Failed to open file for reading");
				return;
			}
			// Read a line from file
			char line[64];
			fgets(line, sizeof(line), f0);

			int pcnt = atoi(line);
			pcnt ++;
			fclose(f0);
			f0 = fopen(pic_cnt, "w");
			fprintf(f0, "%d",pcnt);		
			fclose(f0);

			//以下代码为新建图片文件并写入图片数据
			char *file_hello=malloc(50);
			sprintf(file_hello,"%s/%d.bmp",MOUNT_POINT,pcnt);
			FILE *f = fopen(file_hello, "w");
			free(file_hello);
			if (f == NULL) {
				ESP_LOGE(TAG, "Failed to open file for writing");
				return;
			}
			//uint8_t *bmp_buf = malloc(CAM_WIDE*CAM_HIGH*3+54);
			uint32_t _caps = MALLOC_CAP_8BIT;
        	uint8_t *bmp_buf = (uint8_t *)heap_caps_malloc(CAM_WIDE*CAM_HIGH*3+54, _caps);

			if(bmp_buf == NULL)
				ESP_LOGE(TAG, "Failed to open bmp_buf");

			memcpy(bmp_buf,bmp_head_320,sizeof(bmp_head_320));

			for(int i=0;i<CAM_WIDE*CAM_HIGH;i++)
			{
				uint16_t col = pic->buf[i*2+1] | (pic->buf[i*2]<<8);
				bmp_buf[i*3 + 54] = (col & 0x001F)<<3;
				bmp_buf[i*3 + 55] = (col & 0x07E0)>>3;
				bmp_buf[i*3 + 56] = (col & 0xF800)>>8;
			}
			fwrite(bmp_buf,1, CAM_WIDE*CAM_HIGH*3+54,f);
			//free(bmp_buf);
			fclose(f);
		}

#endif
		 vTaskDelay(500 / portTICK_PERIOD_MS);
	}
}

 
void adc1task(void* arg)
{	
	uint16_t adc_read=0.;
    // initialize ADC
    adc1_config_width(ADC_WIDTH_12Bit);
    adc1_config_channel_atten(KEY_ADC_CHANNLE, ADC_ATTEN_11db);
    while(1){
		//adc_read= ((adc1_get_voltage(KEY_ADC_CHANNLE)*0.985)+217.2);
		adc_read= adc1_get_raw(KEY_ADC_CHANNLE);
        //ESP_LOGE(TAG, "ADC RESULT = %d",adc_read);
		sys_key_handler(adc_read);
        vTaskDelay(20/portTICK_PERIOD_MS);
    }
}

static camera_config_t camera_config = {
    .pin_pwdn = CAM_PIN_PWDN,
    .pin_reset = CAM_PIN_RESET,
    .pin_xclk = CAM_PIN_XCLK,
    .pin_sscb_sda = CAM_PIN_SIOD,
    .pin_sscb_scl = CAM_PIN_SIOC,

    .pin_d7 = CAM_PIN_D7,
    .pin_d6 = CAM_PIN_D6,
    .pin_d5 = CAM_PIN_D5,
    .pin_d4 = CAM_PIN_D4,
    .pin_d3 = CAM_PIN_D3,
    .pin_d2 = CAM_PIN_D2,
    .pin_d1 = CAM_PIN_D1,
    .pin_d0 = CAM_PIN_D0,
    .pin_vsync = CAM_PIN_VSYNC,
    .pin_href = CAM_PIN_HREF,
    .pin_pclk = CAM_PIN_PCLK,

    //XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
    .xclk_freq_hz = 20000000,
    .ledc_timer = LEDC_TIMER_0,
    .ledc_channel = LEDC_CHANNEL_0,

    .pixel_format = PIXFORMAT_RGB565, //YUV422,GRAYSCALE,RGB565,JPEG 
    .frame_size = FRAMESIZE_QVGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG
    //.frame_size = FRAMESIZE_VGA,    //QQVGA-UXGA Do not use sizes above QVGA when not JPEG

    .jpeg_quality = 12, //0-63 lower number means higher quality
    .fb_count = 1,       //if more than one, i2s runs in continuous mode. Use only with JPEG
	.fb_location = CAMERA_FB_IN_PSRAM,
//	.fb_location = CAMERA_FB_IN_DRAM,
    .grab_mode = CAMERA_GRAB_WHEN_EMPTY,
	
};


static esp_err_t init_camera()
{
    //initialize the camera
     esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Camera Init Failed");
        return err;
    }

    return ESP_OK;
}



void app_main(void)
{
    led_pwm_ctrl_config();
    if(ESP_OK != init_camera()) {
        ESP_LOGE(TAG, "Camera Init Failed");
		//return;
    }
	 xTaskCreate(adc1task, "adc1task", 1024*6, NULL, 1, NULL);
	 xTaskCreate(main_task, "main_task", 1024*6, NULL, 2, NULL);
}
