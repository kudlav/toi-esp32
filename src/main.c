#include "main.h"

#include <driver/adc.h>
#define GPIO_PHOTO    36 // IO63

void lightInit() {
	gpio_pad_select_gpio(GPIO_LED_BOARD);
	ESP_ERROR_CHECK(gpio_set_direction(GPIO_LED_BOARD, GPIO_MODE_INPUT));
	adc1_config_width(ADC_WIDTH_BIT_12);
	adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_11); // TODO check and adjust max voltage
}

void hallInit() {
	adc1_config_width(ADC_WIDTH_BIT_12);
}

int hallGet() {
	char TAG[] = "hallGet";
	ESP_LOGI(TAG, "Hall sensor readings (0-4095):");
	int hall = hall_sensor_read();
	ESP_LOGI(TAG, "  %d", hall);
	return hall;
}

void app_main() {
	ESP_ERROR_CHECK(nvs_flash_init());

	wifi_init_sta();
	lightInit();
	hallInit();

	bool tempOk = (tempInit() > 0);
	bool coapOk = coapInit();

	while(tempOk && coapOk) {
        char data[512];

		// Get values
		float temp = getDsTemp(0);
		int hall = hallGet();

        sprintf(data, "tmpRaw=%f;tmpFilter=TODO;lux=TODO;hall=%d;tmpDev=TODO;cpu=TODO;dev=espXKUDLA15", temp, hall);

		ESP_LOGI("main", "Collected data to send: %s", data);

		coapSend((unsigned char *) data);

		vTaskDelay(5000.0 / portTICK_PERIOD_MS);
    }

	coapCleanup();
}
