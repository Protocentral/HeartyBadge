#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include <driver/spi_master.h>
#include <esp_log.h>
#include "nvs_flash.h"
#include "protocentral_apa102.h"

spi_device_handle_t handle;
spi_bus_config_t bus_config;
spi_device_interface_config_t dev_config;
spi_transaction_t trans_desc = { };
unsigned char *data;

const uint8_t heart[144] = {						0,0,0,0,0,0,0,0,
											   0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
		      0,0,0,0,1,1,1,1,1,0,0,0,
				  0,0,0,1,1,1,0,0,0,
						  0,0,1,0		};

const uint8_t heart_small[144] = {						0,0,0,0,0,0,0,0,
											   0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,1,1,1,1,1,0,0,0,0,0,
		      0,0,0,0,0,1,1,1,0,0,0,0,
				  0,0,0,0,1,0,0,0,0,
						  0,0,0,0		};

//doesn't look like a heart, :|
const uint8_t heart_verysmall[144] = {						0,0,0,0,0,0,0,0,
											   0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,00,0,0,0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,1,0,1,0,0,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,1,1,1,0,0,0,0,
		      0,0,0,0,0,0,1,0,0,0,0,0,
				  0,0,0,0,0,0,0,0,0,
						  0,0,0,0		};

const uint8_t heart_big[144] = {						0,0,0,0,0,0,0,0,
											   0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,1,1,1,1,1,1,0,0,0,0,
		      0,0,0,0,1,1,1,1,1,0,0,0,
				  0,0,0,1,1,1,0,0,0,
						  0,0,1,0		};

//heart-line
const uint8_t heart_line[144] = {						0,0,0,0,0,0,0,0,
											   0,0,0,0,0,
	 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	 0,0,0,0,0,0,0,1,1,0,1,1,0,0,0,0,0,0,0,
		0,0,0,0,0,1,0,0,1,0,0,1,0,0,0,0,0,			0,0,0,0,0,0,0,0,0,
        0,0,0,0,1,0,0,0,0,0,1,0,0,0,0,
		      0,0,0,0,1,0,0,0,1,0,0,0,
				  0,0,0,1,0,1,0,0,0,
						  0,0,1,0		};

uint8_t red_residue = 0, green_residue = 50, blue_residue = 80;

void init_apa102_spi(void)
{
	gpio_pad_select_gpio(14);
	gpio_set_direction(14, GPIO_MODE_OUTPUT);
	gpio_set_level(14, 0);
	vTaskDelay(100 / portTICK_PERIOD_MS);
	gpio_set_level(14, 1);

	bus_config.sclk_io_num = CLOCK_PIN; // CLK
	bus_config.mosi_io_num = DATA_PIN; // MOSI
	bus_config.miso_io_num = -1; // MISO (not used)
	bus_config.quadwp_io_num = -1; // Not used
	bus_config.quadhd_io_num = -1; // Not used
	bus_config.max_transfer_sz=320*2+8;

	ESP_ERROR_CHECK(spi_bus_initialize(VSPI_HOST, &bus_config, 1));

	dev_config.address_bits = 0;
	dev_config.command_bits = 0;
	dev_config.dummy_bits = 0;
	dev_config.mode = 0;
	dev_config.duty_cycle_pos = 0;  // 50%
	dev_config.cs_ena_posttrans = 0;
	dev_config.cs_ena_pretrans = 0;
	dev_config.clock_speed_hz=8*1000*1000,               //Clock out at 10 MHz
	dev_config.spics_io_num = -1;
	dev_config.flags = 0;
	dev_config.queue_size = 1;
	dev_config.pre_cb = NULL;
	dev_config.post_cb = NULL;

	ESP_ERROR_CHECK(spi_bus_add_device(VSPI_HOST, &dev_config, &handle));

	trans_desc.addr = 0;
	trans_desc.cmd = 0;
	trans_desc.flags = 0;

	data = heap_caps_malloc(4*4, MALLOC_CAP_DMA);
	trans_desc.length = 4 * 8;
	trans_desc.tx_buffer = data;
}

void apa102_spi_send_start(void)
{
	data[0] = 0x00;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;

	ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void apa102_spi_send_stop(void)
{
	data[0] = 0xFF;
	data[1] = 0xFF;
	data[2] = 0xFF;
	data[3] = 0xFF;
	ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
	vTaskDelay(100 / portTICK_PERIOD_MS);
}

void glow_pattern(uint32_t pattern_color, uint32_t color, const uint8_t * pattern_matrix){
	
	apa102_spi_send_start();

	for(int i=0;i<NUM_LEDS;i++)
	{

		ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
		vTaskDelay(1 / portTICK_PERIOD_MS);

		if(pattern_matrix[i]){
			
			data[0] = 0xff;
			data[1] = (uint8_t) pattern_color;//0x0;
			data[2] = pattern_color >> 8;//0x0;
			data[3] = pattern_color >> 16;//0x15;
		}
			else{

			data[0] = 0xff;
			data[1] = (uint8_t) color;//0x0;
			data[2] =  color >> 8;//0x0;
			data[3] =  color >> 16;

		}
	}
}

void heart_animation(uint8_t red, uint8_t green, uint8_t blue){

	//set the colour for heart	
	uint32_t colour = setrgb_val(red, green, blue);

	//select the colour for remaining portion	
	uint32_t colour_residue = setrgb_val(red_residue, green_residue, blue_residue);

	glow_pattern(colour, colour_residue, heart_small);

	for(int i = 0; i<2; i++){
		if(red && red<235) red +=  20;
		if(green && green<235) green +=  20;
		if(blue && blue<235) blue +=  20;
		colour = setrgb_val(red, green, blue);
		glow_pattern(colour, colour_residue, heart);
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}

	for(int i = 0; i<2; i++){
		if(red) red -= 20;
		if(green) green -= 20;
		if(blue) blue -= 20;
		colour = setrgb_val(red, green, blue);
		glow_pattern(colour, colour_residue, heart);

		vTaskDelay(1 / portTICK_PERIOD_MS);
	}
}

void heart_intensity(uint8_t red, uint8_t green, uint8_t blue){

	//uint8_t red_residue = 35, green_residue = 0, blue_residue = 0;	//background colour
	uint32_t colour_residue = setrgb_val(red_residue, green_residue, blue_residue);

	uint32_t colour = setrgb_val(red, green, blue);
	glow_pattern(colour, colour_residue, heart);
	vTaskDelay(1 / portTICK_PERIOD_MS);
	
	for(int i = 0; i<2; i++){
		if(red && red<235) red +=  24;
		if(green && green<235) green +=  24;
		if(blue && blue<235) blue +=  24;
		colour = setrgb_val(red, green, blue);
		glow_pattern(colour, colour_residue, heart);
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}

	for(int i = 0; i<2; i++){
		if(red) red -= 24;
		if(green) green -= 24;
		if(blue) blue -= 24;
		colour = setrgb_val(red, green, blue);
		glow_pattern(colour, colour_residue, heart);
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}

}

void blink_heart(uint8_t pattern)
{
	uint8_t red = 30, green = 0, blue = 0;		//heart colour

	red_residue = 0, green_residue = 30, blue_residue = 80;


#ifdef INTENSITY
	heart_intensity(red, green, blue);
#else
	heart_animation(red, green, blue);
#endif

}

void blink_apa102(void)
{
	apa102_spi_send_start();

	for(int i=0;i<144;i++)
	{
		ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
		vTaskDelay(1 / portTICK_PERIOD_MS);

		data[0] = 0x09;
		data[1] = 0x0;
		data[2] = 0x15;
		data[3] = 0x0;
	}

	apa102_spi_send_stop();
	vTaskDelay(5 / portTICK_PERIOD_MS);

	apa102_spi_send_start();

	data[0] = 0xFF;
	data[1] = 0x00;
	data[2] = 0x00;
	data[3] = 0x00;

	for(int i=0;i<144;i++)
	{
		ESP_ERROR_CHECK(spi_device_transmit(handle, &trans_desc));
		vTaskDelay(1 / portTICK_PERIOD_MS);
	}

	apa102_spi_send_stop();
	vTaskDelay(300 / portTICK_PERIOD_MS);

}
