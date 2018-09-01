#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "nvs_flash.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "driver/i2c.h"
#include "driver/uart.h"
#include "driver/sdmmc_host.h"

#include "esp_log.h"
#include "max30003.h"
#include "ble.h"
#include "hpadc.h"
#include "protocentral_apa102.h"


#define TAG "HRV-BADGE"
#define delay_ms(ms) vTaskDelay((ms) / portTICK_RATE_MS)
#define KALAM32_MDNS_ENABLED    FALSE
#define KALAM32_MDNS_NAME       "heartypatch"
#define BUF_SIZE  1000

extern QueueHandle_t xQueue_tcp;
extern xSemaphoreHandle print_mux;

unsigned int global_heartRate ; 


void app_main(void)
{

	esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
	
	xQueue_tcp = xQueueCreate(20, sizeof( struct Tcp_Message *));
	if( xQueue_tcp==NULL )
	{
		ESP_LOGI(TAG, "Failed to create Queue..!");
	}
	
    init_apa102_spi();
    blink_apa102();
    
    max30003_initchip(PIN_SPI_MISO,PIN_SPI_MOSI,PIN_SPI_SCK,PIN_SPI_CS);

    kalam_start_max30003();
    kalam32_adc_start();

	vTaskDelay(2000/ portTICK_PERIOD_MS);		//give sometime for max to settle

#ifdef CONFIG_BLE_MODE_ENABLE
	kalam_ble_Init();		
#endif
    while(true){

        vTaskDelay(10/ portTICK_PERIOD_MS);   
    }
	
}
